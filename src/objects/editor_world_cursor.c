#include "editor_world_cursor.h"

#include "../engine.h"
#include "../input_map.h"
#include "../input.h"
#include "../collision.h"

#define OBJ_EDITOR_WORLD_CURSOR_SENS 0.01

#include <math.h>

struct tds_object_type obj_editor_world_cursor_type = {
	.type_name = "obj_editor_world_cursor",
	.default_sprite = "spr_editor_cursor",
	.default_params = 0,
	.default_params_size = 0,
	.data_size = sizeof(struct obj_editor_world_cursor_data),
	.func_init = obj_editor_world_cursor_init,
	.func_destroy = obj_editor_world_cursor_destroy,
	.func_update = obj_editor_world_cursor_update,
	.func_draw = obj_editor_world_cursor_draw,
	.func_msg = (void*) 0,
	.func_import = (void*) 0,
	.func_export = (void*) 0,
	.save = 0
};

void obj_editor_world_cursor_init(struct tds_object* ptr) {
	tds_input_set_mouse(tds_engine_global->input_handle, 0.0f, 0.0f);

	ptr->layer = 10;
}

void obj_editor_world_cursor_destroy(struct tds_object* ptr) {
}

void obj_editor_world_cursor_update(struct tds_object* ptr) {
}

void obj_editor_world_cursor_draw(struct tds_object* ptr) {
	struct obj_editor_world_cursor_data* data = ptr->object_data;

	ptr->x = roundf(tds_engine_global->input_handle->mx * OBJ_EDITOR_WORLD_CURSOR_SENS + tds_engine_global->camera_handle->x);
	ptr->y = roundf(tds_engine_global->input_handle->my * -OBJ_EDITOR_WORLD_CURSOR_SENS + tds_engine_global->camera_handle->y);
}
