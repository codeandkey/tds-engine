#pragma once

#include "../object.h"

struct tds_object_type tds_obj_wall_type;

#define TDS_OBJ_WALL_FRAME_TOP_LEFT_CORNER 0
#define TDS_OBJ_WALL_FRAME_TOP_HALF 1
#define TDS_OBJ_WALL_FRAME_TOP_RIGHT_CORNER 2
#define TDS_OBJ_WALL_FRAME_RIGHT_HALF 3
#define TDS_OBJ_WALL_FRAME_UNUSED 4
#define TDS_OBJ_WALL_FRAME_LEFT_HALF 5
#define TDS_OBJ_WALL_FRAME_BOTTOM_LEFT_CORNER 6
#define TDS_OBJ_WALL_FRAME_BOTTOM_HALF 7
#define TDS_OBJ_WALL_FRAME_BOTTOM_RIGHT_CORNER 8

void tds_obj_wall_init(struct tds_object* ptr);
void tds_obj_wall_draw(struct tds_object* ptr);
void tds_obj_wall_destroy(struct tds_object* ptr);
void tds_obj_wall_update(struct tds_object* ptr);
void tds_obj_wall_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* param);
