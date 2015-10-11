#include "editor_cursor.h"

#include "../engine.h"
#include "../input_map.h"
#include "../input.h"
#include "../collision.h"

#define OBJ_EDITOR_CURSOR_SENS 0.01

struct tds_object_type obj_editor_cursor_type = {
	.type_name = "obj_editor_cursor",
	.default_sprite = "spr_editor_cursor",
	.default_params = 0,
	.default_params_size = 0,
	.data_size = sizeof(struct obj_editor_cursor_data),
	.func_init = obj_editor_cursor_init,
	.func_destroy = obj_editor_cursor_destroy,
	.func_update = obj_editor_cursor_update,
	.func_draw = obj_editor_cursor_draw,
	.func_msg = (void*) 0,
	.func_import = (void*) 0,
	.func_export = (void*) 0,
	.save = 0
};

void obj_editor_cursor_init(struct tds_object* ptr) {
	tds_input_set_mouse(tds_engine_global->input_handle, 0.0f, 0.0f);

	ptr->layer = 10;
}

void obj_editor_cursor_destroy(struct tds_object* ptr) {
}

void obj_editor_cursor_update(struct tds_object* ptr) {
}

void obj_editor_cursor_draw(struct tds_object* ptr) {
	struct obj_editor_cursor_data* data = ptr->object_data;

	ptr->x = tds_engine_global->input_handle->mx * OBJ_EDITOR_CURSOR_SENS + tds_engine_global->camera_handle->x;
	ptr->y = tds_engine_global->input_handle->my * -OBJ_EDITOR_CURSOR_SENS + tds_engine_global->camera_handle->y;

	if (tds_input_map_get_mouse_button_pressed(tds_engine_global->input_map_handle, 0, 0) && !data->drag) {
		/* We find a selector and grab it. */

		struct tds_engine_object_list result = tds_engine_get_object_list_by_type(tds_engine_global, "obj_editor_selector");

		for (int i = 0; i < result.size; ++i) {
			if (tds_collision_get_point_overlap(result.buffer[i], ptr->x, ptr->y)) {
				data->drag = result.buffer[i];
				data->x_offset = result.buffer[i]->x - ptr->x;
				data->y_offset = result.buffer[i]->y - ptr->y;
				break;
			}
		}
	}

	if (tds_input_map_get_mouse_button_released(tds_engine_global->input_map_handle, 0, 0) && data->drag) {
		data->drag = 0;
	}

	int angle_mod = tds_input_map_get_key(tds_engine_global->input_map_handle, GLFW_KEY_LEFT_SHIFT, 0);

	if (data->drag) {
		if (angle_mod) {
			data->drag->angle = atan2(ptr->y - data->drag->y, ptr->x - data->drag->x);
		} else {
			data->drag->x = ptr->x + data->x_offset;
			data->drag->y = ptr->y + data->y_offset;
		}
	}
}
