#include "console.h"
#include "engine.h"
#include "log.h"
#include "memory.h"
#include "input_map.h"
#include "objects/objects.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void _tds_console_execute(struct tds_console* ptr);

#define TDS_CONSOLE_FONT_SIZE 10.0f

struct tds_console* tds_console_create(void) {
	struct tds_console* output = tds_malloc(sizeof(struct tds_console));

	/* We can't control anything but rows and cols, so 1:1 may be hard */

	output->rows = tds_engine_global->display_handle->desc.height / (int) TDS_CONSOLE_FONT_SIZE;
	output->cols = tds_engine_global->display_handle->desc.width / (int) TDS_CONSOLE_FONT_SIZE;

	tds_logf(TDS_LOG_MESSAGE, "Creating console with %d rows and %d cols\n", output->rows, output->cols);

	output->buffers = tds_malloc(sizeof(char*) * output->rows);

	for (int i = 0; i < output->rows; ++i) {
		output->buffers[i] = tds_malloc(output->cols);
	}

	output->curs_row = output->curs_col = 0;
	output->enabled = 0;
	output->input_ind = 0;
	memset(output->input_buf, 0, sizeof output->input_buf / sizeof output->input_buf[0]);

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

void tds_console_resize(struct tds_console* ptr) {
	if (!ptr->buffers) {
		return;
	}

	for (int i = 0; i < ptr->rows; ++i) {
		tds_free(ptr->buffers[i]);
	}

	tds_free(ptr->buffers);

	ptr->rows = tds_engine_global->display_handle->desc.height / (int) TDS_CONSOLE_FONT_SIZE;
	ptr->cols = tds_engine_global->display_handle->desc.width / (int) TDS_CONSOLE_FONT_SIZE;

	tds_logf(TDS_LOG_MESSAGE, "Resizing console to %d rows and %d cols\n", ptr->rows, ptr->cols);

	ptr->buffers = tds_malloc(sizeof(char*) * ptr->rows);

	for (int i = 0; i < ptr->rows; ++i) {
		ptr->buffers[i] = tds_malloc(ptr->cols);
	}

	ptr->curs_row = ptr->curs_col = 0;
	tds_console_print(ptr, "$ ");
}

void tds_console_key_pressed(struct tds_console* ptr, int key) {
	if (key == GLFW_KEY_GRAVE_ACCENT) {
		ptr->enabled = !ptr->enabled;
		return;
	}

	if (key == GLFW_KEY_ENTER) {
		tds_console_print(ptr, "\n");
		_tds_console_execute(ptr);
		tds_console_print(ptr, "$ ");
	}

	if (key == GLFW_KEY_BACKSPACE && ptr->input_ind > 0) {
		ptr->input_buf[--ptr->input_ind] = 0;

		if (--ptr->curs_col < 0) {
			if (ptr->curs_row > 0) {
				ptr->curs_col = ptr->cols - 1;
				ptr->curs_row--;
			} else {
				ptr->curs_col = 0;
			}
		}

		ptr->buffers[ptr->curs_row][ptr->curs_col] = 0;
	}
}

int tds_console_char_pressed(struct tds_console* ptr, unsigned int chr) {
	if (!ptr->enabled) {
		return 0;
	}

	if (!chr) {
		return 0;
	}

	if (chr == '`' || chr == '~') {
		return 0;
	}

	if (!isprint(chr)) {
		return 0;
	}

	if (ptr->input_ind >= TDS_CONSOLE_INPUT_SIZE) {
		return 1;
	}

	ptr->input_buf[ptr->input_ind++] = chr;

	char str[2] = {0};
	str[0] = chr;

	tds_console_print(ptr, str);
	return 1;
}

void tds_console_draw(struct tds_console* ptr) {
	if (!ptr->enabled) {
		return;
	}

	tds_overlay_set_color(tds_engine_global->overlay_handle, 1.0f, 1.0f, 1.0f, 1.0f);

	for (int i = 0; i < ptr->rows; ++i) {
		if (!*(ptr->buffers[i])) {
			continue;
		}

		int len = strlen(ptr->buffers[i]);

		if (len > ptr->cols) {
			len = ptr->cols;
		}

		float w = tds_engine_global->display_handle->desc.width, h = tds_engine_global->display_handle->desc.height;

		float offset = h / (float) ptr->rows;
		tds_overlay_render_text(tds_engine_global->overlay_handle, 0.0f, w, offset * i, h, TDS_CONSOLE_FONT_SIZE, ptr->buffers[i], len, TDS_OVERLAY_SCREENSPACE);
	}
}

void tds_console_print(struct tds_console* ptr, const char* str) {
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
	char* cur_cmd = strtok(ptr->input_buf, " ");

	struct tds_object* selected = NULL, *selector = NULL;

	if (editor_cursor) {
		selector = ((struct obj_editor_cursor_data*) editor_cursor->object_data)->last;

		if (selector) {
			selected = ((struct obj_editor_selector_data*) selector->object_data)->target;
		}
	}

	if (!cur_cmd) {
		goto EXECUTE_CLEANUP;
	} else if (!strcmp(cur_cmd, "echo")) {
		while ( (cur_cmd = strtok(NULL, " ")) ) {
			tds_console_print(ptr, cur_cmd);
			tds_console_print(ptr, " ");
		}

		tds_console_print(ptr, "\n");
	} else if (!strcmp(cur_cmd, "create")) {
		char* type = strtok(NULL, " "), *x = strtok(NULL, " "), *y = strtok(NULL, " ");

		if (!(type && x && y)) {
			tds_console_print(ptr, "invalid number of arguments\n");
			goto EXECUTE_CLEANUP;
		}

		tds_console_print(ptr, "searching for object type : [");
		tds_console_print(ptr, type);
		tds_console_print(ptr, "]\n");

		struct tds_object_type* obj_type = tds_object_type_cache_get(tds_engine_global->otc_handle, type);

		if (!obj_type) {
			tds_console_print(ptr, "object type lookup failed\n");
			goto EXECUTE_CLEANUP;
		}

		tds_console_print(ptr, "located object type : [");
		tds_console_print(ptr, obj_type->type_name);
		tds_console_print(ptr, "]\n");

		float x_f = strtof(x, NULL), y_f = strtof(y, NULL);
		struct tds_object* new_object = tds_object_create(obj_type, tds_engine_global->object_buffer, tds_engine_global->sc_handle, x_f, y_f, 0.0f, NULL);
		tds_console_print(ptr, "created object\n");

		if (tds_editor_get_mode() == TDS_EDITOR_MODE_OBJECTS) {
			tds_editor_add_selector(new_object);
		}
	} else if (!strcmp(cur_cmd, "exit") || !strcmp(cur_cmd, "quit")) {
		tds_engine_terminate(tds_engine_global);
	} else if (!strcmp(cur_cmd, "load")) {
		char* file = strtok(NULL, " ");

		if (!file) {
			tds_console_print(ptr, "usage: load <filename>\n");
			goto EXECUTE_CLEANUP;
		}

		tds_console_print(ptr, "loading from ");
		tds_console_print(ptr, file);
		tds_console_print(ptr, "..\n");

		tds_engine_load(tds_engine_global, file);
	} else if (!strcmp(cur_cmd, "+edit")) {
		tds_console_print(ptr, "creating editor objects\n");
		tds_create_editor_objects();
	} else if (!strcmp(cur_cmd, "-edit")) {
		tds_console_print(ptr, "destroying editor objects\n");
		tds_destroy_editor_objects();
	} else if (!strcmp(cur_cmd, "+draw")) {
		tds_console_print(ptr, "enabling draw\n");
		tds_engine_global->enable_draw = 1;
	} else if (!strcmp(cur_cmd, "-draw")) {
		tds_console_print(ptr, "disabling draw\n");
		tds_engine_global->enable_draw = 0;
	} else if (!strcmp(cur_cmd, "+qdraw")) {
		tds_console_print(ptr, "enabling quadtree draw\n");
		tds_engine_global->enable_quadtree_draw = 1;
	} else if (!strcmp(cur_cmd, "-qdraw")) {
		tds_console_print(ptr, "disabling quadtree draw\n");
		tds_engine_global->enable_quadtree_draw = 0;
	} else if (!strcmp(cur_cmd, "+cdraw")) {
		tds_console_print(ptr, "enabling camera draw\n");
		tds_engine_global->enable_camera_draw = 1;
	} else if (!strcmp(cur_cmd, "-cdraw")) {
		tds_console_print(ptr, "disabling camera draw\n");
		tds_engine_global->enable_camera_draw = 0;
	} else if (!strcmp(cur_cmd, "+overlaydb")) {
		tds_console_print(ptr, "enabling overlay doublebuffering\n");
		tds_engine_global->overlay_handle->enable_doublebuffer = 1;
	} else if (!strcmp(cur_cmd, "-overlaydb")) {
		tds_console_print(ptr, "disabling overlay doublebuffering\n");
		tds_engine_global->overlay_handle->enable_doublebuffer = 0;
	} else if (!strcmp(cur_cmd, "+wire")) {
		tds_console_print(ptr, "enabling wireframe\n");
		tds_engine_global->render_handle->enable_wireframe = 1;
	} else if (!strcmp(cur_cmd, "-wire")) {
		tds_console_print(ptr, "disabling wireframe\n");
		tds_engine_global->render_handle->enable_wireframe = 0;
	} else if (!strcmp(cur_cmd, "+dynlights")) {
		tds_console_print(ptr, "enabling dynamic lights\n");
		tds_engine_global->render_handle->enable_dynlights = 1;
	} else if (!strcmp(cur_cmd, "-dynlights")) {
		tds_console_print(ptr, "disabling dynamic lights\n");
		tds_engine_global->render_handle->enable_dynlights = 0;
	} else if (!strcmp(cur_cmd, "+bloom")) {
		tds_console_print(ptr, "enabling bloom\n");
		tds_engine_global->render_handle->enable_bloom = 1;
	} else if (!strcmp(cur_cmd, "-bloom")) {
		tds_console_print(ptr, "disabling bloom\n");
		tds_engine_global->render_handle->enable_bloom = 0;
	} else if (!strcmp(cur_cmd, "+aabb")) {
		tds_console_print(ptr, "enabling AABB occlusion\n");
		tds_engine_global->render_handle->enable_aabb = 1;
	} else if (!strcmp(cur_cmd, "-aabb")) {
		tds_console_print(ptr, "disabling AABB occlusion\n");
		tds_engine_global->render_handle->enable_aabb = 0;
	} else if (!strcmp(cur_cmd, "+update")) {
		tds_console_print(ptr, "enabling update\n");
		tds_engine_global->enable_update = 1;
	} else if (!strcmp(cur_cmd, "-update")) {
		tds_console_print(ptr, "disabling update\n");
		tds_engine_global->enable_update = 0;
	} else if (!strcmp(cur_cmd, "sethiddenscale")) {
		tds_engine_global->camera_handle->hidden_scale = strtof(strtok(NULL, " "), 0);
		/* Reload the camera matrix. */
		tds_camera_set_raw(tds_engine_global->camera_handle, tds_engine_global->camera_handle->width, tds_engine_global->camera_handle->height, tds_engine_global->camera_handle->x, tds_engine_global->camera_handle->y);
		tds_console_print(ptr, "set hidden camera scale\n");
	} else if (!strcmp(cur_cmd, "setambient")) {
		/* Reload the camera matrix. */
		tds_render_set_ambient_brightness(tds_engine_global->render_handle, strtof(strtok(NULL, " "), 0));
		tds_console_print(ptr, "set ambient brightness\n");
	} else if (!strcmp(cur_cmd, "ipart")) {
		int key = strtol(strtok(NULL, " "), 0, 0), val = strtol(strtok(NULL, " "), 0, 0);

		if (selected) {
			tds_object_set_ipart(selected, key, val);
			tds_console_print(ptr, "set ipart for ");
			tds_console_print(ptr, selected->type_name);
			tds_console_print(ptr, "\n");
		} else {
			tds_console_print(ptr, "no object selected\n");
		}
	} else if (!strcmp(cur_cmd, "upart")) {
		int key = strtol(strtok(NULL, " "), 0, 0);
		unsigned int val = strtol(strtok(NULL, " "), 0, 0);

		if (selected) {
			tds_object_set_upart(selected, key, val);
			tds_console_print(ptr, "set upart for ");
			tds_console_print(ptr, selected->type_name);
			tds_console_print(ptr, "\n");
		} else {
			tds_console_print(ptr, "no object selected\n");
		}
	} else if (!strcmp(cur_cmd, "fpart")) {
		int key = strtol(strtok(NULL, " "), 0, 0);
		float val = strtof(strtok(NULL, " "), 0);

		if (selected) {
			tds_object_set_fpart(selected, key, val);
			tds_console_print(ptr, "set fpart for ");
			tds_console_print(ptr, selected->type_name);
			tds_console_print(ptr, "\n");
		} else {
			tds_console_print(ptr, "no object selected\n");
		}
	} else if (!strcmp(cur_cmd, "spart")) {
		int key = strtol(strtok(NULL, " "), 0, 0);
		char* val = strtok(NULL, " ");

		if (selected) {
			tds_object_set_spart(selected, key, val, strlen(val));
			tds_console_print(ptr, "set spart for ");
			tds_console_print(ptr, selected->type_name);
			tds_console_print(ptr, "\n");
		} else {
			tds_console_print(ptr, "no object selected\n");
		}
	} else {
		tds_console_print(ptr, "unknown command\n");
	}

	EXECUTE_CLEANUP:
	memset(ptr->input_buf, 0, sizeof ptr->input_buf / sizeof ptr->input_buf[0]);
	ptr->input_ind = 0;
}
