#pragma once

#include "../object.h"

struct tds_object_type tds_obj_system_type;

void tds_obj_system_init(struct tds_object* ptr);
void tds_obj_system_update(struct tds_object* ptr);
void tds_obj_system_draw(struct tds_object* ptr);
void tds_obj_system_destroy(struct tds_object* ptr);
void tds_obj_system_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);
