#pragma once

#include "../object.h"

extern struct tds_object_type obj_editor_world_cursor_type;

void obj_editor_world_cursor_init(struct tds_object* ptr);
void obj_editor_world_cursor_update(struct tds_object* ptr);
void obj_editor_world_cursor_draw(struct tds_object* ptr);
void obj_editor_world_cursor_destroy(struct tds_object* ptr);
void obj_editor_world_cursor_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);

struct obj_editor_world_cursor_data {
	struct tds_object* drag;
	float x_offset, y_offset;
	int current_block_id;
};
