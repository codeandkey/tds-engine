#pragma once

#include "../object.h"

struct tds_object_type tds_obj_cursor_type;

void tds_obj_cursor_init(struct tds_object* ptr);
void tds_obj_cursor_draw(struct tds_object* ptr);
void tds_obj_cursor_destroy(struct tds_object* ptr);
void tds_obj_cursor_update(struct tds_object* ptr);
void tds_obj_cursor_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);
