#pragma once

#include "../object.h"

extern struct tds_object_type obj_editor_cursor_type;

void obj_editor_cursor_init(struct tds_object* ptr);
void obj_editor_cursor_update(struct tds_object* ptr);
void obj_editor_cursor_draw(struct tds_object* ptr);
void obj_editor_cursor_destroy(struct tds_object* ptr);

struct obj_editor_cursor_data {
	int unused;
};
