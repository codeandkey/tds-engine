#pragma once

#include "../object.h"

#define TDS_OBJ_BULLET_SPEED 0.3

struct tds_object_type tds_obj_bullet_type;

void tds_obj_bullet_init(struct tds_object* ptr);
void tds_obj_bullet_draw(struct tds_object* ptr);
void tds_obj_bullet_destroy(struct tds_object* ptr);
void tds_obj_bullet_update(struct tds_object* ptr);
void tds_obj_bullet_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);

struct tds_obj_bullet_data {
	float xspeed, yspeed;
};
