#include "console.h"
#include "engine.h"
#include "text.h"
#include "log.h"
#include "memory.h"
#include "input_map.h"

#include <string.h>
#include <stdlib.h>

static void _tds_console_execute(struct tds_console* ptr);

struct tds_console* tds_console_create(void) {
	struct tds_console* output = tds_malloc(sizeof(struct tds_console));

	output->font = tds_sprite_cache_get(tds_engine_global->sc_handle, "font_debug");
	output->rows = tds_engine_global->camera_handle->height / output->font->height;
	output->cols = tds_engine_global->camera_handle->width / output->font->width;

	tds_logf(TDS_LOG_MESSAGE, "Creating console with %d rows and %d cols\n", output->rows, output->cols);

	output->buffers = tds_malloc(sizeof(char*) * output->rows);

	for (int i = 0; i < output->rows; ++i) {
		output->buffers[i] = tds_malloc(output->cols);
	}

	output->curs_row = output->curs_col = 0;

	tds_console_print(output, "$ ");

	return output;
}

void tds_console_free(struct tds_console* ptr) {
	for (int i = 0; i < ptr->rows; ++i) {
		tds_free(ptr->buffers[i]);
	}

	tds_free(ptr->buffers);
	tds_free(ptr);
}

void tds_console_update(struct tds_console* ptr) {
	/* This does NOT need to be called on a delta time. Once a frame is enough. */
	char input_char = 0;

	if (tds_input_map_get_key_pressed(tds_engine_global->input_map_handle, GLFW_KEY_ENTER, 0)) {
		tds_console_print(ptr, "\n");
		_tds_console_execute(ptr);
		tds_console_print(ptr, "$ ");
	}

	input_char = tds_input_map_get_char(tds_engine_global->input_map_handle);

	if (!input_char) {
		return;
	}

	char str[2] = {0};
	str[0] = input_char;

	tds_console_print(ptr, str);
}

void tds_console_draw(struct tds_console* ptr) {
	struct tds_text_batch render_batch = {0};

	render_batch.font = ptr->font;
	render_batch.x = tds_engine_global->camera_handle->x - tds_engine_global->camera_handle->width / 2.0f + ptr->font->width / 2.0f;
	render_batch.y = tds_engine_global->camera_handle->y + tds_engine_global->camera_handle->height / 2.0f + ptr->font->width / 2.0f;
	render_batch.z = 0.0f;

	render_batch.r = render_batch.g = render_batch.b = render_batch.a = 1.0f;
	render_batch.angle = 0.0f;
	render_batch.layer = 1000;

	for (int i = 0; i < ptr->rows; ++i) {
		render_batch.y -= ptr->font->height;
		render_batch.str = ptr->buffers[i];
		render_batch.str_len = ptr->cols;

		if (!*render_batch.str) {
			continue;
		}

		tds_text_submit(tds_engine_global->text_handle, &render_batch);
	}
}

void tds_console_print(struct tds_console* ptr, char* str) {
	if (!str) {
		return;
	}

	int str_len = strlen(str);

	for (int i = 0; i < str_len; ++i) {
		if (str[i] != '\n') {
			ptr->buffers[ptr->curs_row][ptr->curs_col] = str[i];
		}

		if (++ptr->curs_col >= ptr->cols || str[i] == '\n') {
			ptr->curs_col = 0;

			if (++ptr->curs_row >= ptr->rows) {
				for (int i = 0; i < ptr->rows - 1; ++i) {
						memcpy(ptr->buffers[i], ptr->buffers[i + 1], ptr->cols);
				}

				memset(ptr->buffers[ptr->rows - 1], 0, ptr->cols);
				ptr->curs_row = ptr->rows - 1;
			}
		}
	}
}

void _tds_console_execute(struct tds_console* ptr) {
	char* cmd = tds_malloc(ptr->cols);
	memcpy(cmd, ptr->buffers[ptr->curs_row - 1], ptr->cols);

	char* cur_cmd = strtok(cmd, " ");
	cur_cmd = strtok(NULL, " ");

	if (!cur_cmd) {
		return;
	}

	if (!strcmp(cur_cmd, "echo")) {
		while ( (cur_cmd = strtok(NULL, " ")) ) {
			tds_console_print(ptr, cur_cmd);
			tds_console_print(ptr, " ");
		}

		tds_console_print(ptr, "\n");

		return;
	}

	if (!strcmp(cur_cmd, "create")) {
		char* type = strtok(NULL, " "), *x = strtok(NULL, " "), *y = strtok(NULL, " ");

		if (!(type && x && y)) {
			tds_console_print(ptr, "invalid number of arguments\n");
			return;
		}

		tds_console_print(ptr, "searching for object type : [");
		tds_console_print(ptr, type);
		tds_console_print(ptr, "]\n");

		struct tds_object_type* obj_type = tds_object_type_cache_get(tds_engine_global->otc_handle, type);

		if (!obj_type) {
			tds_console_print(ptr, "object type lookup failed\n");
			return;
		}

		tds_console_print(ptr, "located object type : [");
		tds_console_print(ptr, obj_type->type_name);
		tds_console_print(ptr, "]\n");

		float x_f = strtof(x, NULL), y_f = strtof(y, NULL);
		tds_object_create(obj_type, tds_engine_global->object_buffer, tds_engine_global->sc_handle, x_f, y_f, 0.0f, NULL);

		tds_console_print(ptr, "created object\n");
		return;
	}

	if (!strcmp(cur_cmd, "exit") || !strcmp(cur_cmd, "quit")) {
		tds_engine_terminate(tds_engine_global);
		return;
	}

	if (!strcmp(cur_cmd, "save")) {
		char* file = strtok(NULL, " ");

		if (!file) {
			tds_console_print(ptr, "usage: save <filename>\n");
			return;
		}

		tds_engine_save(tds_engine_global, file);

		tds_console_print(ptr, "saved map to ");
		tds_console_print(ptr, file);
		tds_console_print(ptr, "\n");

		return;
	}

	if (!strcmp(cur_cmd, "load")) {
		char* file = strtok(NULL, " ");

		if (!file) {
			tds_console_print(ptr, "usage: load <filename>\n");
			return;
		}

		tds_console_print(ptr, "loading from ");
		tds_console_print(ptr, file);
		tds_console_print(ptr, "..\n");

		tds_engine_load(tds_engine_global, file);
		return;
	}

	tds_free(cmd);
}
