#include "console.h"
#include "engine.h"
#include "text.h"
#include "log.h"
#include "memory.h"
#include "input_map.h"

#include <string.h>

static void _tds_console_execute(struct tds_console* ptr);
static void _tds_console_print(struct tds_console* ptr, char* str);

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

	_tds_console_print(output, "$ ");

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
		_tds_console_execute(ptr);
		_tds_console_print(ptr, "\n$ ");
	}

	input_char = tds_input_map_get_char(tds_engine_global->input_map_handle);

	if (!input_char) {
		return;
	}

	char str[2] = {0};
	str[0] = input_char;

	_tds_console_print(ptr, str);
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

void _tds_console_print(struct tds_console* ptr, char* str) {
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
	memcpy(cmd, ptr->buffers[ptr->curs_row], ptr->cols);

	char* cur_cmd = strtok(cmd, " ");
	cur_cmd = strtok(NULL, " ");

	if (!strcmp(cur_cmd, "echo")) {
		_tds_console_print(ptr, "\n");

		while ( (cur_cmd = strtok(NULL, " ")) ) {
			_tds_console_print(ptr, cur_cmd);
			_tds_console_print(ptr, " ");
		}
	}
}
