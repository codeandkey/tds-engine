#pragma once

#include "../object.h"

struct tds_object_type tds_obj_player_legs_type;

void tds_obj_player_legs_init(struct tds_object* ptr);
void tds_obj_player_legs_update(struct tds_object* ptr);
void tds_obj_player_legs_draw(struct tds_object* ptr);
void tds_obj_player_legs_destroy(struct tds_object* ptr);
void tds_obj_player_legs_msg(struct tds_object* ptr, struct tds_object* caller, int msg, void* p);
