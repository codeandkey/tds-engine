#include "render.h"
#include "log.h"
#include "memory.h"
#include "object.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GLXW/glxw.h>

struct _tds_file {
	char* data;
	int size;
};

static void _tds_render_object(struct tds_render* ptr, struct tds_object* obj, int layer);
static void _tds_render_text_batch(struct tds_render* ptr, struct tds_text_batch* data);
static int _tds_load_shaders(struct tds_render* ptr, const char* vs, const char* fs);
static struct _tds_file _tds_load_file(const char* filename);

struct tds_render* tds_render_create(struct tds_camera* camera, struct tds_handle_manager* hmgr, struct tds_text* text) {
	struct tds_render* output = tds_malloc(sizeof(struct tds_render));

	output->object_buffer = hmgr;
	output->camera_handle = camera;
	output->text_handle = text;

	_tds_load_shaders(output, TDS_RENDER_SHADER_WORLD_VS, TDS_RENDER_SHADER_WORLD_FS);

	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);

	return output;
}

void tds_render_free(struct tds_render* ptr) {
	glDetachShader(ptr->render_program, ptr->render_vs);
	glDetachShader(ptr->render_program, ptr->render_fs);
	glDeleteShader(ptr->render_vs);
	glDeleteShader(ptr->render_fs);
	glDeleteProgram(ptr->render_program);
	tds_free(ptr);
}

void tds_render_clear(struct tds_render* ptr) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	tds_text_clear(ptr->text_handle);
}

void tds_render_draw(struct tds_render* ptr) {
	/* Drawing will be done linearly on a per-layer basis, using a list of occluded objects. */

	if (ptr->object_buffer->max_index <= 0) {
		return;
	}

	int* object_rendered = tds_malloc(ptr->object_buffer->max_index * sizeof(int));
	int* text_rendered = tds_malloc(ptr->text_handle->size * sizeof(int));

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

	int ind = 0; /* Local index for the following loop, we need to index a linked list. */

	for (struct tds_text_batch_entry* i = ptr->text_handle->head; i; i = i->next) {
		text_rendered[ind] = 0;

		if (i->data.layer < min_layer) {
			min_layer = i->data.layer;
		} else if (i->data.layer > max_layer) {
			max_layer = i->data.layer;
		}

		ind++;
	}

	for (int i = min_layer; i <= max_layer; ++i) {
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

		int ind = 0;

		for (struct tds_text_batch_entry* entry = ptr->text_handle->head; entry; entry = entry->next) {
			if (text_rendered[ind]) {
				++ind;
				continue;
			}

			if (i == entry->data.layer) {
				_tds_render_text_batch(ptr, &entry->data);
				text_rendered[ind] = 1;
			}

			++ind;
		}
	}

	tds_free(object_rendered);
	tds_free(text_rendered);
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

void _tds_render_text_batch(struct tds_render* ptr, struct tds_text_batch* data) {
	vec4* sprite_transform = tds_sprite_get_transform(data->font);
	mat4x4 obj_transform, transform_full;

	tds_logf(TDS_LOG_DEBUG, "Rendering text batch [%s] at %f, %f\n", data->str, data->x, data->y);

	/* If we pass the glyph X and Y to the transform function, everything should be taken care of. */

	for (int i = 0; i < data->str_len; ++i) {
		if (!data->str[i]) {
			break;
		}

		vec4* glyph_transform = tds_text_batch_get_transform(data, i);
		mat4x4_mul(obj_transform, glyph_transform, sprite_transform);
		mat4x4_mul(transform_full, ptr->camera_handle->mat_transform, obj_transform);

		glUniform4f(ptr->uniform_color, data->r, data->g, data->b, data->a);
		glUniformMatrix4fv(ptr->uniform_transform, 1, GL_FALSE, (float*) transform_full);

		glBindTexture(GL_TEXTURE_2D, data->font->texture->gl_id);

		glBindVertexArray(data->font->vbo_handle->vao);
		glDrawArrays(data->font->vbo_handle->render_mode, data->str[i] * 6, 6);
	}
}

int _tds_load_shaders(struct tds_render* ptr, const char* vs, const char* fs) {
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
	glUniform4f(ptr->uniform_color, 0.5f, 1.0f, 0.5f, 1.0f);

	mat4x4 identity;
	mat4x4_identity(identity);
	glUniformMatrix4fv(ptr->uniform_transform, 1, GL_FALSE, (float*) *identity);

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
