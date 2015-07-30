#pragma once

#include "../object.h"

#define TDS_OBJ_PLAYER_MAX_SPEED 0.04f
#define TDS_OBJ_PLAYER_ACCEL 0.005f
#define TDS_OBJ_PLAYER_DECEL 1.06f

struct tds_object_type tds_obj_player_type;

void tds_obj_player_init(struct tds_object* ptr);
void tds_obj_player_update(struct tds_object* ptr);
void tds_obj_player_draw(struct tds_object* ptr);
void tds_obj_player_destroy(struct tds_object* ptr);
void tds_obj_player_msg(struct tds_object* ptr, struct tds_object* caller, int msg, void* p);

struct tds_obj_player_data {
	float xspeed, yspeed;
};
