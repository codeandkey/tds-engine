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

	if (tds_input_map_get_key_pressed(tds_engine_global->input_map_handle, GLFW_KEY_EQUAL, 0)) {
		data->current_block_id++;
	}

	if (tds_input_map_get_key_pressed(tds_engine_global->input_map_handle, GLFW_KEY_MINUS, 0)) {
		data->current_block_id--;
	}

	if (data->current_block_id < 0) {
		data->current_block_id = 0;
	}

	struct tds_texture* c_texture = tds_block_map_get(tds_engine_global->block_map_handle, data->current_block_id).texture;

	if (tds_input_map_get_mouse_button(tds_engine_global->input_map_handle, 0, 0)) {
		int block_x = floorf(ptr->x / TDS_WORLD_BLOCK_SIZE) + tds_engine_global->world_handle->width / 2;
		int block_y = floorf(ptr->y / TDS_WORLD_BLOCK_SIZE) + tds_engine_global->world_handle->height / 2;

		tds_world_set_block(tds_engine_global->world_handle, block_x, block_y, data->current_block_id & 0xFF); 
	}

	ptr->x = tds_engine_global->input_handle->mx * OBJ_EDITOR_WORLD_CURSOR_SENS + tds_engine_global->camera_handle->x;
	ptr->y = tds_engine_global->input_handle->my * -OBJ_EDITOR_WORLD_CURSOR_SENS + tds_engine_global->camera_handle->y;

	ptr->x -= fmod(ptr->x, TDS_WORLD_BLOCK_SIZE);
	ptr->y -= fmod(ptr->y, TDS_WORLD_BLOCK_SIZE);

	if (c_texture) {
		ptr->sprite_handle->texture = tds_block_map_get(tds_engine_global->block_map_handle, data->current_block_id).texture;
		ptr->a = 0.3f;
	} else {
		ptr->sprite_handle->texture = tds_texture_cache_get(tds_engine_global->tc_handle, "res/sprites/editor_cursor.png", 32, 32, 0, 0);
		ptr->a = 1.0f;
	}
}
