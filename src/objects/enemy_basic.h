#pragma once

#include "../object.h"

#define TDS_OBJ_ENEMY_AIM_OFFSET (3.141f / 16.0f);

#define TDS_OBJ_ENEMY_STATE_IDLE 0
#define TDS_OBJ_ENEMY_STATE_ALARM 1
#define TDS_OBJ_ENEMY_STATE_SEARCHING 3

#define TDS_OBJ_ENEMY_COOLDOWN 15.0f
#define TDS_OBJ_ENEMY_FIRERATE 0.2f
#define TDS_OBJ_ENEMY_FOV (3.141f / 2.0f)

struct tds_object_type tds_obj_enemy_basic_type;

void tds_obj_enemy_basic_init(struct tds_object* ptr);
void tds_obj_enemy_basic_draw(struct tds_object* ptr);
void tds_obj_enemy_basic_destroy(struct tds_object* ptr);
void tds_obj_enemy_basic_update(struct tds_object* ptr);
void tds_obj_enemy_basic_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);

struct tds_obj_enemy_basic_data {
	int state;
	int state_draw;
	tds_clock_point clock_cooldown, clock_fire;
	float xspeed, yspeed;
};
