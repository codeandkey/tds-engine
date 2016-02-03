#include "render.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "block_map.h"
#include "engine.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GLXW/glxw.h>

struct _tds_file {
	char* data;
	int size;
};

static void _tds_render_object(struct tds_render* ptr, struct tds_object* obj, int layer);
static void _tds_render_world(struct tds_render* ptr, struct tds_world* world);
static void _tds_render_lightmap(struct tds_render* ptr, struct tds_world* world);
static void _tds_render_segments(struct tds_render* ptr, struct tds_world* world, struct tds_camera* cam, int occlude, unsigned int u_transform);
static int _tds_load_world_shaders(struct tds_render* ptr, const char* vs, const char* fs);
static int _tds_load_lightmap_shaders(struct tds_render* ptr, const char* point_gs, const char* dir_gs, const char* point_fs, const char* dir_fs);
static int _tds_load_recomb_shaders(struct tds_render* ptr, const char* recomb_fs_point, const char* recomb_fs_dir);
static int _tds_load_blur_shaders(struct tds_render* ptr, const char* hblur_vs, const char* vblur_vs, const char* blur_fs);
static int _tds_load_bloom_shaders(struct tds_render* ptr, const char* bloom_fs);
static struct _tds_file _tds_load_file(const char* filename);

struct tds_render* tds_render_create(struct tds_camera* camera, struct tds_handle_manager* hmgr) {
	struct tds_render* output = tds_malloc(sizeof(struct tds_render));

	output->object_buffer = hmgr;
	output->camera_handle = camera;

	_tds_load_world_shaders(output, TDS_RENDER_SHADER_WORLD_VS, TDS_RENDER_SHADER_WORLD_FS);
	_tds_load_lightmap_shaders(output, TDS_RENDER_SHADER_POINT_GS, TDS_RENDER_SHADER_DIR_GS, TDS_RENDER_SHADER_POINT_FS, TDS_RENDER_SHADER_DIR_FS);
	_tds_load_recomb_shaders(output, TDS_RENDER_SHADER_RECOMB_FS_POINT, TDS_RENDER_SHADER_RECOMB_FS_DIR);
	_tds_load_blur_shaders(output, TDS_RENDER_SHADER_HBLUR_VS, TDS_RENDER_SHADER_VBLUR_VS, TDS_RENDER_SHADER_BLUR_FS);
	_tds_load_bloom_shaders(output, TDS_RENDER_SHADER_BLOOM_FS);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);

	unsigned int display_width = tds_engine_global->display_handle->desc.width, display_height = tds_engine_global->display_handle->desc.height;
	output->lightmap_rt = tds_rt_create(display_width, display_height);
	output->dir_rt = tds_rt_create(display_width, display_height);
	output->point_rt = tds_rt_create(TDS_RENDER_POINT_RT_SIZE, TDS_RENDER_POINT_RT_SIZE);

	output->post_rt1 = tds_rt_create(display_width, display_height);
	output->post_rt2 = tds_rt_create(display_width, display_height);

	tds_rt_bind(NULL);

	return output;
}

void tds_render_free(struct tds_render* ptr) {
	glUseProgram(0);

	glDetachShader(ptr->render_program_point, ptr->render_vs);
	glDetachShader(ptr->render_program_point, ptr->render_pgs);
	glDetachShader(ptr->render_program_point, ptr->render_pfs);
	glDeleteShader(ptr->render_pgs);
	glDeleteShader(ptr->render_pfs);
	glDeleteProgram(ptr->render_program_point);

	glDetachShader(ptr->render_program_dir, ptr->render_vs);
	glDetachShader(ptr->render_program_dir, ptr->render_dgs);
	glDetachShader(ptr->render_program_dir, ptr->render_dfs);
	glDeleteShader(ptr->render_dgs);
	glDeleteShader(ptr->render_dfs);
	glDeleteProgram(ptr->render_program_dir);

	glDetachShader(ptr->render_program_recomb_point, ptr->render_vs);
	glDetachShader(ptr->render_program_recomb_point, ptr->render_rpfs);
	glDeleteShader(ptr->render_rpfs);
	glDeleteProgram(ptr->render_program_recomb_point);

	glDetachShader(ptr->render_program_recomb_dir, ptr->render_vs);
	glDetachShader(ptr->render_program_recomb_dir, ptr->render_rdfs);
	glDeleteShader(ptr->render_rdfs);
	glDeleteProgram(ptr->render_program_recomb_dir);

	glDetachShader(ptr->render_program, ptr->render_vs);
	glDetachShader(ptr->render_program, ptr->render_fs);
	glDeleteShader(ptr->render_vs);
	glDeleteShader(ptr->render_fs);
	glDeleteProgram(ptr->render_program);

	tds_rt_free(ptr->lightmap_rt);
	tds_rt_free(ptr->dir_rt);
	tds_rt_free(ptr->point_rt);
	tds_rt_free(ptr->post_rt1);
	tds_rt_free(ptr->post_rt2);
	tds_render_clear_lights(ptr);
	tds_free(ptr);
}

void tds_render_clear(struct tds_render* ptr) {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void tds_render_draw(struct tds_render* ptr, struct tds_world* world, struct tds_overlay* overlay) {
	/* Drawing will be done linearly on a per-layer basis, using a list of occluded objects. */
	int render_objects = 1;

	if (ptr->object_buffer->max_index <= 0) {
		render_objects = 0;
	}

	int* object_rendered = NULL;

	if (render_objects) {
		object_rendered = tds_malloc(ptr->object_buffer->max_index * sizeof(int));
	}

	int min_layer = 0;
	int max_layer = 0;

	for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
		struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[i].data;
		object_rendered[i] = 0;

		if (!target) {
			object_rendered[i] = 1;
			continue;
		}

		if (!target->visible || !target->sprite_handle) {
			object_rendered[i] = 1;
			continue;
		}

		if (target->layer < min_layer) {
			min_layer = target->layer;
		} else if (target->layer > max_layer) {
			max_layer = target->layer;
		}
	}

	tds_rt_bind(ptr->post_rt1); /* Bind the post RT, we will be doing some post-processing */
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = min_layer; i <= max_layer; ++i) {
		if (!i) {
			/* We render the world at depth 0. */
			_tds_render_world(ptr, world);
		}

		for (int j = 0; j < ptr->object_buffer->max_index; ++j) {
			if (object_rendered[j]) {
				continue;
			}

			struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[j].data;

			if (target->layer == i) {
				object_rendered[j] = 1;
				_tds_render_object(ptr, target, i);
			}
		}
	}

	if (object_rendered) {
		tds_free(object_rendered);
	}

	_tds_render_lightmap(ptr, world);

	struct tds_vertex verts[] = {
		{-1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
		{1.0f, -1.0f, 0.0f, 1.0f, 0.0f},
		{1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
		{-1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
		{1.0f, -1.0f, 0.0f, 1.0f, 0.0f},
		{-1.0f, -1.0f, 0.0f, 0.0f, 0.0f}
	};

	struct tds_vertex_buffer* vb_square = tds_vertex_buffer_create(verts, sizeof verts / sizeof *verts, GL_TRIANGLES);

	mat4x4 ident;
	mat4x4_identity(ident);

	tds_rt_bind(ptr->post_rt1); // _tds_render_lightmap changes the RT, reset it here

	glUseProgram(ptr->render_program);
	glBindVertexArray(vb_square->vao);
	glBindTexture(GL_TEXTURE_2D, ptr->lightmap_rt->gl_tex);
	glUniformMatrix4fv(ptr->uniform_transform, 1, GL_FALSE, (float*) *ident);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDrawArrays(vb_square->render_mode, 0, 6);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* The world and lightmaps are done rendering.. we perform post-processing before the OSD */
	/* First, a gaussian hblur */

	tds_rt_bind(NULL); /* First, we render to the screen with the normal world info. */
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(ptr->render_program);
	glBindVertexArray(vb_square->vao);
	glBindTexture(GL_TEXTURE_2D, ptr->post_rt1->gl_tex);
	glUniformMatrix4fv(ptr->uniform_transform, 1, GL_FALSE, (float*) *ident);

	glBlendFunc(GL_ONE, GL_ZERO); /* Copy the textures to preserve alpha. */
	glDrawArrays(vb_square->render_mode, 0, 6);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tds_rt_bind(ptr->post_rt2);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(ptr->render_program_bloom);
	glBindVertexArray(vb_square->vao);
	glBindTexture(GL_TEXTURE_2D, ptr->post_rt1->gl_tex);
	glUniformMatrix4fv(ptr->bl_uniform_transform, 1, GL_FALSE, (float*) *ident);

	glBlendFunc(GL_ONE, GL_ZERO); /* Copy the textures to preserve alpha. */
	glDrawArrays(vb_square->render_mode, 0, 6);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tds_rt_bind(ptr->post_rt1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(ptr->render_program_hblur);
	glBindVertexArray(vb_square->vao);
	glBindTexture(GL_TEXTURE_2D, ptr->post_rt2->gl_tex);
	glUniformMatrix4fv(ptr->hb_uniform_transform, 1, GL_FALSE, (float*) *ident);
	glDrawArrays(vb_square->render_mode, 0, 6);

	tds_rt_bind(ptr->post_rt2);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(ptr->render_program_vblur);
	glBindVertexArray(vb_square->vao);
	glBindTexture(GL_TEXTURE_2D, ptr->post_rt1->gl_tex);
	glUniformMatrix4fv(ptr->vb_uniform_transform, 1, GL_FALSE, (float*) *ident);
	glDrawArrays(vb_square->render_mode, 0, 6);

	tds_rt_bind(NULL); /* We don't clear the NULL rt, it already has world data */

	glUseProgram(ptr->render_program);
	glBindVertexArray(vb_square->vao);
	glBindTexture(GL_TEXTURE_2D, ptr->post_rt2->gl_tex);
	glUniformMatrix4fv(ptr->uniform_transform, 1, GL_FALSE, (float*) *ident);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDrawArrays(vb_square->render_mode, 0, 6);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Overlay is drawn over everything else (including the lightmap) */
	/* We get the image data and dump it into a texture here. We can also reuse the square VB from the lightmap blending. */

	glBindTexture(GL_TEXTURE_2D, tds_overlay_update_texture(overlay));
	glDrawArrays(vb_square->render_mode, 0, 6);

	tds_vertex_buffer_free(vb_square);
}

void _tds_render_object(struct tds_render* ptr, struct tds_object* obj, int layer) {
	/* Grab the sprite VBO, compose the render transform, and send the data to the shaders. */

	vec4* sprite_transform = tds_sprite_get_transform(obj->sprite_handle);
	vec4* object_transform = tds_object_get_transform(obj);

	mat4x4 transform, obj_transform_full;
	mat4x4_mul(obj_transform_full, object_transform, sprite_transform);
	mat4x4_mul(transform, ptr->camera_handle->mat_transform, obj_transform_full);

	glUniform4f(ptr->uniform_color, obj->r, obj->g, obj->b, obj->a);
	glUniformMatrix4fv(ptr->uniform_transform, 1, GL_FALSE, (float*) *transform);

	glBindTexture(GL_TEXTURE_2D, obj->sprite_handle->texture->gl_id);

	glBindVertexArray(obj->sprite_handle->vbo_handle->vao);
	glDrawArrays(obj->sprite_handle->vbo_handle->render_mode, obj->current_frame * 6, 6);
}

int _tds_load_world_shaders(struct tds_render* ptr, const char* vs, const char* fs) {
	int result = 0;

	/* First, load the shader file content into memory. */
	struct _tds_file vs_file = _tds_load_file(vs), fs_file = _tds_load_file(fs);

	ptr->render_vs = glCreateShader(GL_VERTEX_SHADER);
	ptr->render_fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(ptr->render_vs, 1, (const char**) &vs_file.data, (const int*) &vs_file.size);
	glShaderSource(ptr->render_fs, 1, (const char**) &fs_file.data, (const int*) &fs_file.size);

	tds_free(vs_file.data);
	tds_free(fs_file.data);

	glCompileShader(ptr->render_vs);
	glGetShaderiv(ptr->render_vs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile vertex shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_vs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glCompileShader(ptr->render_fs);
	glGetShaderiv(ptr->render_fs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile fragment shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_fs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	ptr->render_program = glCreateProgram();

	glAttachShader(ptr->render_program, ptr->render_vs);
	glAttachShader(ptr->render_program, ptr->render_fs);
	glLinkProgram(ptr->render_program);

	glGetProgramiv(ptr->render_program, GL_LINK_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to link shader program.\n");

		char buf[1024];
		glGetProgramInfoLog(ptr->render_program, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glUseProgram(ptr->render_program);

	ptr->uniform_texture = glGetUniformLocation(ptr->render_program, "tds_texture");
	ptr->uniform_color = glGetUniformLocation(ptr->render_program, "tds_color");
	ptr->uniform_transform = glGetUniformLocation(ptr->render_program, "tds_transform");

	if (ptr->uniform_texture < 0) {
		tds_logf(TDS_LOG_WARNING, "Texture uniform not found in shader.\n");
	}

	if (ptr->uniform_color < 0) {
		tds_logf(TDS_LOG_WARNING, "Color uniform not found in shader.\n");
	}

	if (ptr->uniform_transform < 0) {
		tds_logf(TDS_LOG_WARNING, "Transform uniform not found in shader.\n");
	}

	glUniform1i(ptr->uniform_texture, 0);
	glUniform4f(ptr->uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);

	mat4x4 identity;
	mat4x4_identity(identity);
	glUniformMatrix4fv(ptr->uniform_transform, 1, GL_FALSE, (float*) *identity);

	return 1;
}

int _tds_load_bloom_shaders(struct tds_render* ptr, const char* bloom_fs) {
	int result = 0;

	/* First, load the shader file content into memory. */
	struct _tds_file fs_file = _tds_load_file(bloom_fs);

	ptr->render_blfs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(ptr->render_blfs, 1, (const char**) &fs_file.data, (const int*) &fs_file.size);

	tds_free(fs_file.data);

	glCompileShader(ptr->render_blfs);
	glGetShaderiv(ptr->render_blfs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile vertex shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_blfs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	ptr->render_program_bloom = glCreateProgram();

	glAttachShader(ptr->render_program_bloom, ptr->render_vs);
	glAttachShader(ptr->render_program_bloom, ptr->render_blfs);
	glLinkProgram(ptr->render_program_bloom);

	glGetProgramiv(ptr->render_program_bloom, GL_LINK_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to link bloom shader program.\n");

		char buf[1024];
		glGetProgramInfoLog(ptr->render_program_bloom, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glUseProgram(ptr->render_program_bloom);

	ptr->bl_uniform_texture = glGetUniformLocation(ptr->render_program_bloom, "tds_texture");
	ptr->bl_uniform_color = glGetUniformLocation(ptr->render_program_bloom, "tds_color");
	ptr->bl_uniform_transform = glGetUniformLocation(ptr->render_program_bloom, "tds_transform");

	if (ptr->bl_uniform_texture < 0) {
		tds_logf(TDS_LOG_WARNING, "Texture uniform not found in bloom shader.\n");
	}

	if (ptr->bl_uniform_color < 0) {
		tds_logf(TDS_LOG_WARNING, "Color uniform not found in bloom shader.\n");
	}

	if (ptr->bl_uniform_transform < 0) {
		tds_logf(TDS_LOG_WARNING, "Transform uniform not found in bloom shader.\n");
	}

	glUniform1i(ptr->bl_uniform_texture, 0);
	glUniform4f(ptr->bl_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);

	mat4x4 identity;
	mat4x4_identity(identity);
	glUniformMatrix4fv(ptr->bl_uniform_transform, 1, GL_FALSE, (float*) *identity);

	return 1;
}

int _tds_load_recomb_shaders(struct tds_render* ptr, const char* recomb_fs_point, const char* recomb_fs_dir) {
	int result = 0;

	struct _tds_file dfs_file = _tds_load_file(recomb_fs_dir);
	struct _tds_file pfs_file = _tds_load_file(recomb_fs_point);

	ptr->render_rpfs = glCreateShader(GL_FRAGMENT_SHADER);
	ptr->render_rdfs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(ptr->render_rpfs, 1, (const char**) &pfs_file.data, (const int*) &pfs_file.size);
	glShaderSource(ptr->render_rdfs, 1, (const char**) &dfs_file.data, (const int*) &dfs_file.size);

	tds_free(pfs_file.data);
	tds_free(dfs_file.data);

	glCompileShader(ptr->render_rpfs);
	glGetShaderiv(ptr->render_rpfs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile recombination point shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_rpfs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glCompileShader(ptr->render_rdfs);
	glGetShaderiv(ptr->render_rdfs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile recombination directional shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_rdfs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	ptr->render_program_recomb_dir = glCreateProgram();
	ptr->render_program_recomb_point = glCreateProgram();

	glAttachShader(ptr->render_program_recomb_point, ptr->render_vs);
	glAttachShader(ptr->render_program_recomb_point, ptr->render_rpfs);
	glLinkProgram(ptr->render_program_recomb_point);

	glGetProgramiv(ptr->render_program_recomb_point, GL_LINK_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to link shader program.\n");

		char buf[1024];
		glGetProgramInfoLog(ptr->render_program_recomb_point, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glAttachShader(ptr->render_program_recomb_dir, ptr->render_vs);
	glAttachShader(ptr->render_program_recomb_dir, ptr->render_rdfs);
	glLinkProgram(ptr->render_program_recomb_dir);

	glGetProgramiv(ptr->render_program_recomb_dir, GL_LINK_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to link shader program.\n");

		char buf[1024];
		glGetProgramInfoLog(ptr->render_program_recomb_dir, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glUseProgram(ptr->render_program_recomb_point);

	ptr->rp_uniform_texture = glGetUniformLocation(ptr->render_program_recomb_point, "tds_texture");
	ptr->rp_uniform_color = glGetUniformLocation(ptr->render_program_recomb_point, "tds_color");
	ptr->rp_uniform_transform = glGetUniformLocation(ptr->render_program_recomb_point, "tds_transform");

	if (ptr->rp_uniform_texture < 0) {
		tds_logf(TDS_LOG_WARNING, "Texture uniform not found in point shader.\n");
	}

	if (ptr->rp_uniform_color < 0) {
		tds_logf(TDS_LOG_WARNING, "Color uniform not found in point shader.\n");
	}

	if (ptr->rp_uniform_transform < 0) {
		tds_logf(TDS_LOG_WARNING, "Transform uniform not found in point shader.\n");
	}

	glUniform1i(ptr->rp_uniform_texture, 0);
	glUniform4f(ptr->rp_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);

	mat4x4 identity;
	mat4x4_identity(identity);
	glUniformMatrix4fv(ptr->rp_uniform_transform, 1, GL_FALSE, (float*) *identity);

	glUseProgram(ptr->render_program_recomb_dir);

	ptr->rd_uniform_texture = glGetUniformLocation(ptr->render_program_recomb_dir, "tds_texture");
	ptr->rd_uniform_color = glGetUniformLocation(ptr->render_program_recomb_dir, "tds_color");
	ptr->rd_uniform_transform = glGetUniformLocation(ptr->render_program_recomb_dir, "tds_transform");

	if (ptr->rd_uniform_texture < 0) {
		tds_logf(TDS_LOG_WARNING, "Texture uniform not found in directional shader.\n");
	}

	if (ptr->rd_uniform_color < 0) {
		tds_logf(TDS_LOG_WARNING, "Color uniform not found in directional shader.\n");
	}

	if (ptr->rd_uniform_transform < 0) {
		tds_logf(TDS_LOG_WARNING, "Transform uniform not found in directional shader.\n");
	}

	glUniform1i(ptr->rd_uniform_texture, 0);
	glUniform4f(ptr->rd_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);

	mat4x4_identity(identity);
	glUniformMatrix4fv(ptr->rd_uniform_transform, 1, GL_FALSE, (float*) *identity);

	return 1;
}

int _tds_load_blur_shaders(struct tds_render* ptr, const char* hblur_vs, const char* vblur_vs, const char* blur_fs) {
	int result = 0;

	struct _tds_file hvs_file = _tds_load_file(hblur_vs);
	struct _tds_file vvs_file = _tds_load_file(vblur_vs);
	struct _tds_file bfs_file = _tds_load_file(blur_fs);

	ptr->render_hbvs = glCreateShader(GL_VERTEX_SHADER);
	ptr->render_vbvs = glCreateShader(GL_VERTEX_SHADER);
	ptr->render_bfs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(ptr->render_hbvs, 1, (const char**) &hvs_file.data, (const int*) &hvs_file.size);
	glShaderSource(ptr->render_vbvs, 1, (const char**) &vvs_file.data, (const int*) &vvs_file.size);
	glShaderSource(ptr->render_bfs, 1, (const char**) &bfs_file.data, (const int*) &bfs_file.size);

	tds_free(hvs_file.data);
	tds_free(vvs_file.data);
	tds_free(bfs_file.data);

	glCompileShader(ptr->render_hbvs);
	glGetShaderiv(ptr->render_hbvs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile hblur shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_hbvs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glCompileShader(ptr->render_vbvs);
	glGetShaderiv(ptr->render_vbvs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile vblur shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_vbvs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glCompileShader(ptr->render_bfs);
	glGetShaderiv(ptr->render_bfs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile blur fragment shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_bfs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	ptr->render_program_hblur = glCreateProgram();
	ptr->render_program_vblur = glCreateProgram();

	glAttachShader(ptr->render_program_hblur, ptr->render_hbvs);
	glAttachShader(ptr->render_program_hblur, ptr->render_bfs);
	glLinkProgram(ptr->render_program_hblur);

	glAttachShader(ptr->render_program_vblur, ptr->render_vbvs);
	glAttachShader(ptr->render_program_vblur, ptr->render_bfs);
	glLinkProgram(ptr->render_program_vblur);

	glGetProgramiv(ptr->render_program_hblur, GL_LINK_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to link hblur shader program.\n");

		char buf[1024];
		glGetProgramInfoLog(ptr->render_program_hblur, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glGetProgramiv(ptr->render_program_vblur, GL_LINK_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to link vblur shader program.\n");

		char buf[1024];
		glGetProgramInfoLog(ptr->render_program_vblur, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glUseProgram(ptr->render_program_hblur);

	ptr->hb_uniform_texture = glGetUniformLocation(ptr->render_program_hblur, "tds_texture");
	ptr->hb_uniform_color = glGetUniformLocation(ptr->render_program_hblur, "tds_color");
	ptr->hb_uniform_transform = glGetUniformLocation(ptr->render_program_hblur, "tds_transform");

	if (ptr->hb_uniform_texture < 0) {
		tds_logf(TDS_LOG_WARNING, "Texture uniform not found in hblur shader.\n");
	}

	if (ptr->hb_uniform_color < 0) {
		tds_logf(TDS_LOG_WARNING, "Color uniform not found in hblur shader.\n");
	}

	if (ptr->hb_uniform_transform < 0) {
		tds_logf(TDS_LOG_WARNING, "Transform uniform not found in hblur shader.\n");
	}

	glUniform1i(ptr->hb_uniform_texture, 0);
	glUniform4f(ptr->hb_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);

	mat4x4 identity;
	mat4x4_identity(identity);
	glUniformMatrix4fv(ptr->hb_uniform_transform, 1, GL_FALSE, (float*) *identity);

	glUseProgram(ptr->render_program_vblur);

	ptr->vb_uniform_texture = glGetUniformLocation(ptr->render_program_vblur, "tds_texture");
	ptr->vb_uniform_color = glGetUniformLocation(ptr->render_program_vblur, "tds_color");
	ptr->vb_uniform_transform = glGetUniformLocation(ptr->render_program_vblur, "tds_transform");

	if (ptr->vb_uniform_texture < 0) {
		tds_logf(TDS_LOG_WARNING, "Texture uniform not found in vblur shader.\n");
	}

	if (ptr->vb_uniform_color < 0) {
		tds_logf(TDS_LOG_WARNING, "Color uniform not found in vblur shader.\n");
	}

	if (ptr->vb_uniform_transform < 0) {
		tds_logf(TDS_LOG_WARNING, "Transform uniform not found in vblur shader.\n");
	}

	glUniform1i(ptr->vb_uniform_texture, 0);
	glUniform4f(ptr->vb_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);
	glUniformMatrix4fv(ptr->vb_uniform_transform, 1, GL_FALSE, (float*) *identity);

	glUseProgram(0);

	return 1;
}

int _tds_load_lightmap_shaders(struct tds_render* ptr, const char* point_gs, const char* dir_gs, const char* point_fs, const char* dir_fs) {
	int result = 0;

	struct _tds_file pgs_file = _tds_load_file(point_gs), pfs_file = _tds_load_file(point_fs);
	struct _tds_file dgs_file = _tds_load_file(dir_gs), dfs_file = _tds_load_file(dir_fs);

	ptr->render_pgs = glCreateShader(GL_GEOMETRY_SHADER);
	ptr->render_pfs = glCreateShader(GL_FRAGMENT_SHADER);
	ptr->render_dgs = glCreateShader(GL_GEOMETRY_SHADER);
	ptr->render_dfs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(ptr->render_pgs, 1, (const char**) &pgs_file.data, (const int*) &pgs_file.size);
	glShaderSource(ptr->render_pfs, 1, (const char**) &pfs_file.data, (const int*) &pfs_file.size);
	glShaderSource(ptr->render_dgs, 1, (const char**) &dgs_file.data, (const int*) &dgs_file.size);
	glShaderSource(ptr->render_dfs, 1, (const char**) &dfs_file.data, (const int*) &dfs_file.size);

	tds_free(pgs_file.data);
	tds_free(pfs_file.data);
	tds_free(dgs_file.data);
	tds_free(dfs_file.data);

	glCompileShader(ptr->render_pgs);
	glGetShaderiv(ptr->render_pgs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile point geometry shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_pgs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glCompileShader(ptr->render_pfs);
	glGetShaderiv(ptr->render_pfs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile point fragment shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_pfs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glCompileShader(ptr->render_dgs);
	glGetShaderiv(ptr->render_dgs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile directional geometry shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_dgs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glCompileShader(ptr->render_dfs);
	glGetShaderiv(ptr->render_dfs, GL_COMPILE_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to compile directional fragment shader.\n");

		char buf[1024];
		glGetShaderInfoLog(ptr->render_dfs, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	ptr->render_program_point = glCreateProgram();
	ptr->render_program_dir = glCreateProgram();

	glAttachShader(ptr->render_program_point, ptr->render_vs);
	glAttachShader(ptr->render_program_point, ptr->render_pgs);
	glAttachShader(ptr->render_program_point, ptr->render_pfs);
	glLinkProgram(ptr->render_program_point);

	glAttachShader(ptr->render_program_dir, ptr->render_vs);
	glAttachShader(ptr->render_program_dir, ptr->render_dgs);
	glAttachShader(ptr->render_program_dir, ptr->render_dfs);
	glLinkProgram(ptr->render_program_dir);

	glGetProgramiv(ptr->render_program_point, GL_LINK_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to link point shader program.\n");

		char buf[1024];
		glGetProgramInfoLog(ptr->render_program_point, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glGetProgramiv(ptr->render_program_dir, GL_LINK_STATUS, &result);

	if (!result) {
		tds_logf(TDS_LOG_WARNING, "Failed to link directional shader program.\n");

		char buf[1024];
		glGetProgramInfoLog(ptr->render_program_dir, 1024, NULL, buf);
		tds_logf(TDS_LOG_CRITICAL, "Error log : %s\n", buf);
	}

	glUseProgram(ptr->render_program_point);

	ptr->p_uniform_texture = glGetUniformLocation(ptr->render_program_point, "tds_texture");
	ptr->p_uniform_color = glGetUniformLocation(ptr->render_program_point, "tds_color");
	ptr->p_uniform_transform = glGetUniformLocation(ptr->render_program_point, "tds_transform");

	if (ptr->p_uniform_texture < 0) {
		tds_logf(TDS_LOG_WARNING, "Texture uniform not found in point shader.\n");
	}

	if (ptr->p_uniform_color < 0) {
		tds_logf(TDS_LOG_WARNING, "Color uniform not found in point shader.\n");
	}

	if (ptr->p_uniform_transform < 0) {
		tds_logf(TDS_LOG_WARNING, "Transform uniform not found in point shader.\n");
	}

	glUniform1i(ptr->p_uniform_texture, 0);
	glUniform4f(ptr->p_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);

	mat4x4 identity;
	mat4x4_identity(identity);
	glUniformMatrix4fv(ptr->p_uniform_transform, 1, GL_FALSE, (float*) *identity);

	glUseProgram(ptr->render_program_dir);

	ptr->d_uniform_texture = glGetUniformLocation(ptr->render_program_dir, "tds_texture");
	ptr->d_uniform_color = glGetUniformLocation(ptr->render_program_dir, "tds_color");
	ptr->d_uniform_transform = glGetUniformLocation(ptr->render_program_dir, "tds_transform");
	ptr->d_uniform_dir = glGetUniformLocation(ptr->render_program_dir, "light_dir");

	if (ptr->d_uniform_texture < 0) {
		tds_logf(TDS_LOG_WARNING, "Texture uniform not found in directional shader.\n");
	}

	if (ptr->d_uniform_color < 0) {
		tds_logf(TDS_LOG_WARNING, "Color uniform not found in directional shader.\n");
	}

	if (ptr->d_uniform_transform < 0) {
		tds_logf(TDS_LOG_WARNING, "Transform uniform not found in directional shader.\n");
	}

	glUniform1i(ptr->d_uniform_texture, 0);
	glUniform4f(ptr->d_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);
	glUniformMatrix4fv(ptr->d_uniform_transform, 1, GL_FALSE, (float*) *identity);

	glUseProgram(0);

	return 1;
}

struct _tds_file _tds_load_file(const char* filename) {
	struct _tds_file output;

	output.data = (void*) 0;
	output.size = 0;

	FILE* inp = fopen(filename, "r");

	if (!inp) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to open %s for reading\n", filename);
		return output;
	}

	fseek(inp, 0, SEEK_END);
	output.size = ftell(inp);
	fseek(inp, 0, SEEK_SET);

	output.data = tds_malloc(output.size);

	fread(output.data, 1, output.size, inp);
	fclose(inp);

	return output;
}

void _tds_render_world(struct tds_render* ptr, struct tds_world* world) {
	/* We want to render each h-block and wrap the texcoord accordingly.
	 * To effectively wrap world blocks, the textures will have to be in their own files.
	 * The block map has O(1) access complexity, so we can query textures all the time without worry.
	 * World hblocks must have their own VBOs and generating them on a per-frame basis would seriously hit performance.
	 * The hblock VBOs are accessible as a part of the hblock structure. */

	struct tds_world_hblock* cur = world->block_list_head;

	while (cur) {
		struct tds_block_type render_type = tds_block_map_get(tds_engine_global->block_map_handle, cur->id);

		if (!render_type.texture) {
			tds_logf(TDS_LOG_WARNING, "Block type %d does not have an associated texture.\n", cur->id);
			cur = cur->next;
			continue;
		}

		/* The translation must take into account that the block size may not be aligned. */
		float render_x = TDS_WORLD_BLOCK_SIZE * (cur->x - world->width / 2.0f + (cur->w - 1) / 2.0f);
		float render_y = TDS_WORLD_BLOCK_SIZE * (cur->y - world->height / 2.0f);

		/* We implement a quick AABB test with the world block. */
		float block_left = render_x - cur->w / 2.0f * TDS_WORLD_BLOCK_SIZE;
		float block_right = render_x + cur->w / 2.0f * TDS_WORLD_BLOCK_SIZE;
		float block_top = render_y + TDS_WORLD_BLOCK_SIZE / 2.0f;
		float block_bottom = render_y - TDS_WORLD_BLOCK_SIZE / 2.0f;

		if (block_right < ptr->camera_handle->x - ptr->camera_handle->width / 2.0f) {
			cur = cur->next;
			continue;
		}

		if (block_left > ptr->camera_handle->x + ptr->camera_handle->width / 2.0f) {
			cur = cur->next;
			continue;
		}

		if (block_top < ptr->camera_handle->y - ptr->camera_handle->height / 2.0f) {
			cur = cur->next;
			continue;
		}

		if (block_bottom > ptr->camera_handle->y + ptr->camera_handle->height / 2.0f) {
			cur = cur->next;
			continue;
		}

		mat4x4 transform, transform_full;
		mat4x4_identity(transform); // This may be unnecessary.
		mat4x4_translate(transform, render_x, render_y, 0.0f);
		mat4x4_mul(transform_full, ptr->camera_handle->mat_transform, transform);

		glUniform4f(ptr->uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);
		glUniformMatrix4fv(ptr->uniform_transform, 1, GL_FALSE, (float*) *transform_full);

		glBindTexture(GL_TEXTURE_2D, render_type.texture->gl_id);

		glBindVertexArray(cur->vb->vao);
		glDrawArrays(cur->vb->render_mode, 0, 6);

		cur = cur->next;
	}
}

void _tds_render_segments(struct tds_render* ptr, struct tds_world* world, struct tds_camera* cam, int occlude, unsigned int u_transform) {
	struct tds_world_segment* cur = world->segment_list;

	while (cur) {
		/* Since the segments are absolutely positioned, we are saved a ton of time. The final transformation matrix really is just the camera. */

		if (occlude) {
			if (cur->x1 < cam->x - cam->width / 2.0f && cur->x2 < cam->x - cam->width / 2.0f) {
				cur = cur->next;
				continue;
			}

			if (cur->x1 > cam->x + cam->width / 2.0f && cur->x2 > cam->x + cam->width / 2.0f) {
				cur = cur->next;
				continue;
			}

			if (cur->y1 < cam->y - cam->height / 2.0f && cur->y2 < cam->y - cam->height / 2.0f) {
				cur = cur->next;
				continue;
			}

			if (cur->y1 > cam->y + cam->height / 2.0f && cur->y2 < cam->y + cam->height / 2.0f) {
				cur = cur->next;
				continue;
			}
		}

		glUniformMatrix4fv(u_transform, 1, GL_FALSE, (float*) *(cam->mat_transform));
		glBindVertexArray(cur->vb->vao);
		glDrawArrays(cur->vb->render_mode, 0, 2);

		cur = cur->next;
	}
}

void _tds_render_lightmap(struct tds_render* ptr, struct tds_world* world) {
	/* This function should fill the ptr->lightmap_rt with the world's light information. */
	/* We render each light individually and construct the shadowmap. */

	tds_rt_bind(ptr->lightmap_rt);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	tds_rt_bind(NULL);

	/* We will use a basic [-1:1] square VBO to do most of the light rendering.
	 * It will come in handy later. */

	struct tds_vertex verts[] = {
		{-1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
		{1.0f, -1.0f, 0.0f, 1.0f, 0.0f},
		{1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
		{-1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
		{1.0f, -1.0f, 0.0f, 1.0f, 0.0f},
		{-1.0f, -1.0f, 0.0f, 0.0f, 0.0f}
	};

	struct tds_vertex_buffer* vb_square = tds_vertex_buffer_create(verts, sizeof verts / sizeof *verts, GL_TRIANGLES);
	mat4x4 point_light_transform;

	struct tds_render_light* cur = ptr->light_list;
	struct tds_camera* cam_point = tds_camera_create(tds_engine_global->display_handle), *cam_dir = ptr->camera_handle;
	struct tds_camera* cam_use = cam_point;

	while (cur) {
		switch(cur->type) {
		case TDS_RENDER_LIGHT_POINT:
			glUseProgram(ptr->render_program_point);
			tds_rt_bind(ptr->point_rt);
			tds_camera_set_raw(cam_point, cur->dist * 2.0f, cur->dist * 2.0f, cur->x, cur->y);
			cam_use = cam_point;
			break;
		case TDS_RENDER_LIGHT_DIRECTIONAL:
			glUseProgram(ptr->render_program_dir);
			glUniform2f(ptr->d_uniform_dir, cur->x, cur->y);
			tds_rt_bind(ptr->dir_rt);
			cam_use = cam_dir;
			break;
		}

		glClearColor(cur->r, cur->g, cur->b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		_tds_render_segments(ptr, world, cam_use, (cam_use == cam_point) ? 1 : 0, (cam_use == cam_point) ? ptr->p_uniform_transform : ptr->d_uniform_transform);

		/*
		 * Per-light rendering : mini lightmaps for each light, before being combined into final screen lightmap
		 * To render the point lights, we set a special camera ortho before rendering with the pointlight shader.
		 * To render directional lights, we render with the normal camera ortho.
		 * We can occlude with point lights as we know the casting distance. We cannot occlude any segments with directional lighting.
		 */

		switch (cur->type) {
		case TDS_RENDER_LIGHT_POINT:
			glUseProgram(ptr->render_program_recomb_point);
			break;
		case TDS_RENDER_LIGHT_DIRECTIONAL:
			glUseProgram(ptr->render_program_recomb_dir);
			break;
		}

		tds_rt_bind(ptr->lightmap_rt);

		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

		mat4x4 pt_final, ident;
		mat4x4_identity(ident);

		switch(cur->type) {
		case TDS_RENDER_LIGHT_POINT:
			{
				struct tds_vertex verts[] = {
					{-cur->dist, cur->dist, 0.0f, 0.0f, 1.0f},
					{cur->dist, -cur->dist, 0.0f, 1.0f, 0.0f},
					{cur->dist, cur->dist, 0.0f, 1.0f, 1.0f},
					{-cur->dist, cur->dist, 0.0f, 0.0f, 1.0f},
					{cur->dist, -cur->dist, 0.0f, 1.0f, 0.0f},
					{-cur->dist, -cur->dist, 0.0f, 0.0f, 0.0f}
				};

				struct tds_vertex_buffer* vb_point = tds_vertex_buffer_create(verts, sizeof verts / sizeof *verts, GL_TRIANGLES);
				mat4x4_translate(point_light_transform, cur->x, cur->y, 0.0f);
				mat4x4_mul(pt_final, ptr->camera_handle->mat_transform, point_light_transform);
				glUniformMatrix4fv(ptr->rp_uniform_transform, 1, GL_FALSE, (float*) *pt_final);
				glBindVertexArray(vb_point->vao);
				glBindTexture(GL_TEXTURE_2D, ptr->point_rt->gl_tex);
				glDrawArrays(vb_point->render_mode, 0, 6);
				tds_vertex_buffer_free(vb_point);
			}
			break;
		case TDS_RENDER_LIGHT_DIRECTIONAL:
			glUniformMatrix4fv(ptr->rd_uniform_transform, 1, GL_FALSE, (float*) *ident);
			glBindVertexArray(vb_square->vao);
			glBindTexture(GL_TEXTURE_2D, ptr->dir_rt->gl_tex);
			glDrawArrays(vb_square->render_mode, 0, 6);
			break;
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		tds_rt_bind(NULL);
		cur = cur->next;
	}

	tds_vertex_buffer_free(vb_square);
	tds_camera_free(cam_point);
}

void tds_render_submit_light(struct tds_render* ptr, struct tds_render_light lt) {
	struct tds_render_light* new_light = tds_malloc(sizeof *new_light);

	*new_light = lt;
	new_light->next = ptr->light_list;
	ptr->light_list = new_light;
}

void tds_render_clear_lights(struct tds_render* ptr) {
	struct tds_render_light* head = ptr->light_list, *tmp = NULL;

	while (head) {
		tmp = head->next;
		tds_free(head);
		head = tmp;
	}

	ptr->light_list = NULL;
}
