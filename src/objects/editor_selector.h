#pragma once

/* Gives the selector a target to follow. */
#define OBJ_EDITOR_SELECTOR_MSG_TARGET 0

#include "../object.h"

void obj_editor_selector_init(struct tds_object* ptr);
void obj_editor_selector_destroy(struct tds_object* ptr);
void obj_editor_selector_update(struct tds_object* ptr);
void obj_editor_selector_draw(struct tds_object* ptr);
void obj_editor_selector_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);

struct tds_object_type obj_editor_selector_type;

struct obj_editor_selector_data {
	struct tds_object* target;
};
