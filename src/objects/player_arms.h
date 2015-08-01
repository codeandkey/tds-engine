#pragma once

#include "../object.h"

struct tds_object_type tds_obj_player_arms_type;

void tds_obj_player_arms_init(struct tds_object* ptr);
void tds_obj_player_arms_draw(struct tds_object* ptr);
void tds_obj_player_arms_destroy(struct tds_object* ptr);
void tds_obj_player_arms_update(struct tds_object* ptr);
void tds_obj_player_arms_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);

struct tds_obj_player_arms_data {
	int flag_swing;
};
