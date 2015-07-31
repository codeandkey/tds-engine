#pragma once

#include "../object.h"

struct tds_object_type tds_obj_player_type;

void tds_obj_player_init(struct tds_object* ptr);
void tds_obj_player_update(struct tds_object* ptr);
void tds_obj_player_draw(struct tds_object* ptr);
void tds_obj_player_destroy(struct tds_object* ptr);
void tds_obj_player_msg(struct tds_object* ptr, struct tds_object* caller, int msg, void* p);
