#pragma once

#include "../object.h"

struct tds_object_type tds_obj_enemy_basic_type;

void tds_obj_enemy_basic_init(struct tds_object* ptr);
void tds_obj_enemy_basic_draw(struct tds_object* ptr);
void tds_obj_enemy_basic_destroy(struct tds_object* ptr);
void tds_obj_enemy_basic_update(struct tds_object* ptr);
void tds_obj_enemy_basic_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);

struct tds_obj_enemy_basic_data {
	int unused;
};