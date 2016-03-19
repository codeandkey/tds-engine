#include "render.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "block_map.h"
#include "engine.h"

#include <stdlib.h>
#include <math.h>
#include <GLXW/glxw.h>

static void _tds_render_object(struct tds_render* ptr, struct tds_object* obj, int layer, struct tds_shader* shader);
static void _tds_render_world(struct tds_render* ptr, struct tds_world* world);
static void _tds_render_hblock_callback(void* world_ptr, void* hblock_ptr);
static void _tds_render_lightmap(struct tds_render* ptr, struct tds_world* world);
static void _tds_render_segments(struct tds_render* ptr, struct tds_world* world, struct tds_camera* cam, int occlude, struct tds_shader* shader);
static void _tds_render_background(struct tds_render* ptr, struct tds_bg* bg);

struct tds_render* tds_render_create(struct tds_camera* camera, struct tds_handle_manager* hmgr) {
	struct tds_render* output = tds_malloc(sizeof(struct tds_render));

	output->object_buffer = hmgr;
	output->camera_handle = camera;

	output->shader_passthrough = tds_shader_create(TDS_RENDER_SHADER_WORLD_VS, NULL, TDS_RENDER_SHADER_WORLD_FS);
	output->shader_light_point = tds_shader_create(TDS_RENDER_SHADER_WORLD_VS, TDS_RENDER_SHADER_POINT_GS, TDS_RENDER_SHADER_POINT_FS);
	output->shader_light_dir = tds_shader_create(TDS_RENDER_SHADER_WORLD_VS, TDS_RENDER_SHADER_DIR_GS, TDS_RENDER_SHADER_DIR_FS);
	output->shader_recomb_point = tds_shader_create(TDS_RENDER_SHADER_WORLD_VS, NULL, TDS_RENDER_SHADER_RECOMB_FS_POINT);
	output->shader_recomb_dir = tds_shader_create(TDS_RENDER_SHADER_WORLD_VS, NULL, TDS_RENDER_SHADER_RECOMB_FS_DIR);
	output->shader_hblur = tds_shader_create(TDS_RENDER_SHADER_HBLUR_VS, NULL, TDS_RENDER_SHADER_BLUR_FS);
	output->shader_vblur = tds_shader_create(TDS_RENDER_SHADER_VBLUR_VS, NULL, TDS_RENDER_SHADER_BLUR_FS);
	output->shader_bloom = tds_shader_create(TDS_RENDER_SHADER_WORLD_VS, NULL, TDS_RENDER_SHADER_BLOOM_FS);

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
	output->post_rt3 = tds_rt_create(display_width, display_height);

	output->enable_bloom = 1;
	output->enable_dynlights = 1;
	output->enable_aabb = 1;
	output->enable_wireframe = 0;
	output->fade_factor = 1.0f;

	tds_render_set_ambient_brightness(output, 0.5f);

	tds_rt_bind(NULL);

	return output;
}

void tds_render_free(struct tds_render* ptr) {
	tds_shader_free(ptr->shader_passthrough);
	tds_shader_free(ptr->shader_light_point);
	tds_shader_free(ptr->shader_light_dir);
	tds_shader_free(ptr->shader_recomb_point);
	tds_shader_free(ptr->shader_recomb_dir);
	tds_shader_free(ptr->shader_bloom);
	tds_shader_free(ptr->shader_hblur);
	tds_shader_free(ptr->shader_vblur);

	tds_rt_free(ptr->lightmap_rt);
	tds_rt_free(ptr->dir_rt);
	tds_rt_free(ptr->point_rt);
	tds_rt_free(ptr->post_rt1);
	tds_rt_free(ptr->post_rt2);
	tds_rt_free(ptr->post_rt3);
	tds_render_clear_lights(ptr);
	tds_free(ptr);
}

void tds_render_clear(struct tds_render* ptr) {
	tds_rt_bind(ptr->post_rt1);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	tds_rt_bind(ptr->post_rt2);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	tds_rt_bind(ptr->post_rt3);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void tds_render_set_ambient_brightness(struct tds_render* ptr, float brightness) {
	ptr->ambient_r = ptr->ambient_g = ptr->ambient_b = brightness;
}

void tds_render_set_ambient_color(struct tds_render* ptr, float r, float g, float b) {
	ptr->ambient_r = r;
	ptr->ambient_g = g;
	ptr->ambient_b = b;
}

void tds_render_draw(struct tds_render* ptr, struct tds_world** world_list, int world_count, struct tds_render_flat* flat_world, struct tds_render_flat* flat_overlay) {
	/* Drawing will be done linearly on a per-layer basis, using a list of occluded objects. */
	int render_objects = 1;

	if (ptr->enable_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (ptr->object_buffer->max_index <= 0) {
		render_objects = 0;
	}

	int* object_rendered = NULL;

	if (render_objects) {
		object_rendered = tds_malloc(ptr->object_buffer->max_index * sizeof(int));
	}

	int min_layer = 0;
	int max_layer = world_count; /* Make sure to at least render all of the world layers. */

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

	tds_profile_push(tds_engine_global->profile_handle, "BG/Object/FX render");
	tds_shader_bind(ptr->shader_passthrough);

	_tds_render_background(ptr, tds_engine_global->bg_handle);
	tds_effect_render(tds_engine_global->effect_handle, ptr->shader_passthrough);

	for (int i = min_layer; i <= max_layer; ++i) {
		if (i < world_count) {
			/* We render the world at depth 0. */
			_tds_render_world(ptr, world_list[i]);
		}

		for (int j = 0; j < ptr->object_buffer->max_index; ++j) {
			if (object_rendered[j]) {
				continue;
			}

			struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[j].data;

			if (target->layer == i) {
				object_rendered[j] = 1;
				_tds_render_object(ptr, target, i, ptr->shader_passthrough);
			}
		}
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); /* Even if the user wants wireframe, don't do post-processing on wireframe. */

	tds_profile_pop(tds_engine_global->profile_handle);

	if (object_rendered) {
		tds_free(object_rendered);
	}

	if (ptr->enable_dynlights && world_count) {
		_tds_render_lightmap(ptr, tds_engine_get_foreground_world(tds_engine_global));
	}

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

	tds_profile_push(tds_engine_global->profile_handle, "Post-processing");

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	tds_rt_bind(ptr->post_rt1);
	tds_shader_bind(ptr->shader_passthrough);

	/* Render flat world backbuf over game fb. */
	tds_shader_set_transform(ptr->shader_passthrough, (float*) *ident);
	tds_shader_set_color(ptr->shader_passthrough, 1.0f, 1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, flat_world->rt_backbuf->gl_tex);
	glDrawArrays(vb_square->render_mode, 0, vb_square->vertex_count);

	if (ptr->enable_dynlights) {
		tds_rt_bind(ptr->post_rt2); // _tds_render_lightmap changes the RT, reset it here
		glClear(GL_COLOR_BUFFER_BIT);

		// The world is rendered in RT1, we will hblur the lightmap to RT2 and then vblur it back to RT1

		tds_shader_bind(ptr->shader_hblur);
		glBindVertexArray(vb_square->vao);
		glBindTexture(GL_TEXTURE_2D, ptr->lightmap_rt->gl_tex);
		tds_shader_set_transform(ptr->shader_hblur, (float*) *ident);
		tds_shader_set_color(ptr->shader_hblur, 1.0f, 1.0f, 1.0f, 1.0f);

		glDrawArrays(vb_square->render_mode, 0, 6);

		// vblur RT2 to RT1 (lightmap composition)
		tds_rt_bind(ptr->post_rt1);

		tds_shader_bind(ptr->shader_vblur);
		glBindVertexArray(vb_square->vao);
		glBindTexture(GL_TEXTURE_2D, ptr->post_rt2->gl_tex);
		tds_shader_set_transform(ptr->shader_vblur, (float*) *ident);
		tds_shader_set_color(ptr->shader_vblur, 1.0f, 1.0f, 1.0f, 1.0f);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
		glDrawArrays(vb_square->render_mode, 0, 6);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	tds_rt_bind(NULL); /* First, we render to the screen with the normal world info. */
	glClear(GL_COLOR_BUFFER_BIT);

	tds_shader_bind(ptr->shader_passthrough);
	glBindVertexArray(vb_square->vao);
	glBindTexture(GL_TEXTURE_2D, ptr->post_rt1->gl_tex);
	tds_shader_set_transform(ptr->shader_passthrough, (float*) *ident);
	tds_shader_set_color(ptr->shader_passthrough, 1.0f, 1.0f, 1.0f, ptr->fade_factor);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(vb_square->render_mode, 0, 6);

	/* flat overlay rendering */
	tds_shader_set_color(ptr->shader_passthrough, 1.0f, 1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, flat_overlay->rt_backbuf->gl_tex);
	glDrawArrays(vb_square->render_mode, 0, vb_square->vertex_count);

	/* post-processing */

	if (ptr->enable_bloom) {
		tds_rt_bind(ptr->post_rt2);
		glClear(GL_COLOR_BUFFER_BIT);

		tds_shader_bind(ptr->shader_bloom);
		glBindVertexArray(vb_square->vao);
		glBindTexture(GL_TEXTURE_2D, ptr->post_rt1->gl_tex);
		tds_shader_set_transform(ptr->shader_bloom, (float*) *ident);
		tds_shader_set_color(ptr->shader_bloom, 1.0f, 1.0f, 1.0f, 1.0f);

		glBlendFunc(GL_ONE, GL_ZERO); /* Copy the textures to preserve alpha. */
		glDrawArrays(vb_square->render_mode, 0, 6);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		tds_rt_bind(ptr->post_rt1);
		glClear(GL_COLOR_BUFFER_BIT);

		tds_shader_bind(ptr->shader_hblur);
		glBindVertexArray(vb_square->vao);
		glBindTexture(GL_TEXTURE_2D, ptr->post_rt2->gl_tex);
		tds_shader_set_transform(ptr->shader_hblur, (float*) *ident);
		tds_shader_set_color(ptr->shader_hblur, 1.0f, 1.0f, 1.0f, 1.0f);
		glDrawArrays(vb_square->render_mode, 0, 6);

		tds_rt_bind(ptr->post_rt2);
		glClear(GL_COLOR_BUFFER_BIT);

		tds_shader_bind(ptr->shader_vblur);
		glBindVertexArray(vb_square->vao);
		glBindTexture(GL_TEXTURE_2D, ptr->post_rt1->gl_tex);
		tds_shader_set_transform(ptr->shader_vblur, (float*) *ident);
		tds_shader_set_color(ptr->shader_vblur, 1.0f, 1.0f, 1.0f, 1.0f);
		glDrawArrays(vb_square->render_mode, 0, 6);

		tds_rt_bind(NULL); /* We don't clear the NULL rt, it already has world data */

		tds_shader_bind(ptr->shader_passthrough);
		glBindVertexArray(vb_square->vao);
		glBindTexture(GL_TEXTURE_2D, ptr->post_rt2->gl_tex);
		tds_shader_set_transform(ptr->shader_passthrough, (float*) *ident);
		tds_shader_set_color(ptr->shader_passthrough, 1.0f, 1.0f, 1.0f, 1.0f);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDrawArrays(vb_square->render_mode, 0, 6);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	tds_profile_pop(tds_engine_global->profile_handle);

	tds_vertex_buffer_free(vb_square);
}

void _tds_render_object(struct tds_render* ptr, struct tds_object* obj, int layer, struct tds_shader* shader) {
	/* Grab the sprite VBO, compose the render transform, and send the data to the shaders. */

	vec4* sprite_transform = tds_sprite_get_transform(obj->sprite_handle);
	vec4* object_transform = tds_object_get_transform(obj);

	mat4x4 transform, obj_transform_full;
	mat4x4_mul(obj_transform_full, object_transform, sprite_transform);
	mat4x4_mul(transform, ptr->camera_handle->mat_transform, obj_transform_full);

	tds_shader_set_color(shader, obj->r, obj->g, obj->b, obj->a);
	tds_shader_set_transform(shader, (float*) *transform);

	glBindTexture(GL_TEXTURE_2D, obj->sprite_handle->texture->gl_id);

	glBindVertexArray(obj->sprite_handle->vbo_handle->vao);
	glDrawArrays(obj->sprite_handle->vbo_handle->render_mode, obj->current_frame * 6, 6);
}

void _tds_render_world(struct tds_render* ptr, struct tds_world* world) {
	float camera_left = tds_engine_global->camera_handle->x - tds_engine_global->camera_handle->width / 2.0f;
	float camera_right = tds_engine_global->camera_handle->x + tds_engine_global->camera_handle->width / 2.0f;
	float camera_top = tds_engine_global->camera_handle->y + tds_engine_global->camera_handle->height / 2.0f;
	float camera_bottom = tds_engine_global->camera_handle->y - tds_engine_global->camera_handle->height / 2.0f;

	tds_quadtree_walk(world->quadtree, camera_left, camera_right, camera_top, camera_bottom, world, _tds_render_hblock_callback);
}

void _tds_render_hblock_callback(void* usr, void* data) {
	struct tds_render* ptr = tds_engine_global->render_handle;
	struct tds_world* world = (struct tds_world*) usr;
	struct tds_world_hblock* cur = data;

	/* The translation must take into account that the block size may not be aligned. */
	float render_x = TDS_WORLD_BLOCK_SIZE * (cur->x - world->width / 2.0f + (cur->w - 1) / 2.0f);
	float render_y = TDS_WORLD_BLOCK_SIZE * (cur->y - world->height / 2.0f);

	if (ptr->enable_aabb) {
		float camera_left = tds_engine_global->camera_handle->x - tds_engine_global->camera_handle->width / 2.0f;
		float camera_right = tds_engine_global->camera_handle->x + tds_engine_global->camera_handle->width / 2.0f;
		float camera_top = tds_engine_global->camera_handle->y + tds_engine_global->camera_handle->height / 2.0f;
		float camera_bottom = tds_engine_global->camera_handle->y - tds_engine_global->camera_handle->height / 2.0f;

		float block_left = render_x - cur->w / 2.0f * TDS_WORLD_BLOCK_SIZE;
		float block_right = render_x + cur->w / 2.0f * TDS_WORLD_BLOCK_SIZE;
		float block_top = render_y + TDS_WORLD_BLOCK_SIZE / 2.0f;
		float block_bottom = render_y - TDS_WORLD_BLOCK_SIZE / 2.0f;

		if (block_left > camera_right || block_right < camera_left || block_bottom > camera_top || block_top < camera_bottom) {
			return;
		}
	}

	struct tds_block_type render_type = tds_block_map_get(tds_engine_global->block_map_handle, cur->id);

	if (!render_type.texture) {
		tds_logf(TDS_LOG_WARNING, "Block type %d does not have an associated texture.\n", cur->id);
		return;
	}

	mat4x4 transform, transform_full;
	mat4x4_identity(transform); // This may be unnecessary.
	mat4x4_translate(transform, render_x, render_y, 0.0f);
	mat4x4_mul(transform_full, ptr->camera_handle->mat_transform, transform);

	tds_shader_bind(ptr->shader_passthrough);
	tds_shader_set_color(ptr->shader_passthrough, 1.0f, 1.0f, 1.0f, 1.0f);
	tds_shader_set_transform(ptr->shader_passthrough, (float*) *transform_full);

	glBindTexture(GL_TEXTURE_2D, render_type.texture->gl_id);

	glBindVertexArray(cur->vb->vao);
	glDrawArrays(cur->vb->render_mode, 0, 6);
}

void _tds_render_segments(struct tds_render* ptr, struct tds_world* world, struct tds_camera* cam, int occlude, struct tds_shader* shader) {
	tds_shader_set_transform(shader, (float*) *(cam->mat_transform));
	glBindVertexArray(world->segment_vb->vao);
	glDrawArrays(world->segment_vb->render_mode, 0, world->segment_vb->vertex_count);
}

void _tds_render_lightmap(struct tds_render* ptr, struct tds_world* world) {
	/* This function should fill the ptr->lightmap_rt with the world's light information. */
	/* We render each light individually and construct the shadowmap. */

	tds_rt_bind(ptr->lightmap_rt);

	glClearColor(ptr->ambient_r, ptr->ambient_g, ptr->ambient_b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
			tds_shader_bind(ptr->shader_light_point);
			tds_rt_bind(ptr->point_rt);
			tds_camera_set_raw(cam_point, cur->dist * 2.0f, cur->dist * 2.0f, cur->x, cur->y);
			cam_use = cam_point;
			break;
		case TDS_RENDER_LIGHT_DIRECTIONAL:
			tds_shader_bind(ptr->shader_light_dir);
			tds_rt_bind(ptr->dir_rt);
			cam_use = cam_dir;
			break;
		}

		if (cur->type == TDS_RENDER_LIGHT_POINT) {
			if (cur->x - cur->dist > cam_dir->x + cam_dir->width / 2.0f || cur->x + cur->dist < cam_dir->x - cam_dir->width / 2.0f || cur->y - cur->dist > cam_dir->y + cam_dir->height / 2.0f || cur->y + cur->dist < cam_dir->y - cam_dir->height / 2.0f) {
				cur = cur->next;
				continue;
			}
		}

		glClearColor(cur->r, cur->g, cur->b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		_tds_render_segments(ptr, world, cam_use, (cam_use == cam_point) ? 1 : 0, (cam_use == cam_point) ? ptr->shader_light_point : ptr->shader_light_dir);

		/*
		 * Per-light rendering : mini lightmaps for each light, before being combined into final screen lightmap
		 * To render the point lights, we set a special camera ortho before rendering with the pointlight shader.
		 * To render directional lights, we render with the normal camera ortho.
		 * We can occlude with point lights as we know the casting distance. We cannot occlude any segments with directional lighting.
		 */

		switch (cur->type) {
		case TDS_RENDER_LIGHT_POINT:
			tds_shader_bind(ptr->shader_recomb_point);
			break;
		case TDS_RENDER_LIGHT_DIRECTIONAL:
			tds_shader_bind(ptr->shader_recomb_dir);
			break;
		}

		tds_rt_bind(ptr->lightmap_rt);

		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);

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
				tds_shader_set_transform(ptr->shader_recomb_point, (float*) *pt_final);
				glBindVertexArray(vb_point->vao);
				glBindTexture(GL_TEXTURE_2D, ptr->point_rt->gl_tex);
				glDrawArrays(vb_point->render_mode, 0, 6);
				tds_vertex_buffer_free(vb_point);
			}
			break;
		case TDS_RENDER_LIGHT_DIRECTIONAL:
			tds_shader_set_transform(ptr->shader_recomb_dir, (float*) *ident);
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

void _tds_render_background(struct tds_render* ptr, struct tds_bg* bg) {
	/*
	 * We walk each layer and perform the positioning/texture rendering
	 */

	for (int i = 0; i < TDS_BG_LAYERS; ++i) {
		struct tds_bg_entry* cur = bg->layers[i];
		float factor = 1.0f - (float) i / (float) (TDS_BG_LAYERS - 1);

		while (cur) {
			/* We alter the placement of the background by changing the texcoords of the VBO. */

			float cx = tds_engine_global->camera_handle->x, cy = tds_engine_global->camera_handle->y; /* quick camera vars */
			float c_left = cx - tds_engine_global->camera_handle->width / 2.0f, c_right = c_left + tds_engine_global->camera_handle->width;
			float c_bottom = cy - tds_engine_global->camera_handle->height / 2.0f, c_top = c_bottom + tds_engine_global->camera_handle->height;

			float b_size = c_top - c_bottom; // size background based on camera

			float dx, dy;

			if (cur->factor_x) {
				dx = factor * cx;
			} else {
				dx = cx;
			}

			if (cur->factor_y) {
				dy = factor * cy; /* Position of background origin in cameraspace */
			} else {
				dy = cy;
			}

			float tx_left = 0.5f - (dx - c_left) / b_size, tx_right = 0.5f + (c_right - dx) / b_size;
			float tx_bottom = 0.5f - (dy - c_bottom) / b_size, tx_top = 0.5f + (c_top - dy) / b_size;

			struct tds_vertex verts[] = {
				{-1.0f, 1.0f, 0.0f, tx_left, tx_top},
				{1.0f, -1.0f, 0.0f, tx_right, tx_bottom},
				{1.0f, 1.0f, 0.0f, tx_right, tx_top},
				{-1.0f, 1.0f, 0.0f, tx_left, tx_top},
				{1.0f, -1.0f, 0.0f, tx_right, tx_bottom},
				{-1.0f, -1.0f, 0.0f, tx_left, tx_bottom}
			};

			mat4x4 id;
			mat4x4_identity(id);

			tds_shader_bind(ptr->shader_passthrough);

			struct tds_vertex_buffer* vb_point = tds_vertex_buffer_create(verts, sizeof verts / sizeof *verts, GL_TRIANGLES);
			tds_shader_set_transform(ptr->shader_passthrough, (float*) *id);
			glBindVertexArray(vb_point->vao);

			glBindTexture(GL_TEXTURE_2D, cur->tex->gl_id);
			glDrawArrays(vb_point->render_mode, 0, 6);

			tds_vertex_buffer_free(vb_point);
			cur = cur->next;
		}
	}
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

void tds_render_set_fade_factor(struct tds_render* ptr, float fade_factor) {
	ptr->fade_factor = fade_factor;
}
