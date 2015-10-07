#pragma once

#include "object.h"

int tds_collision_get_overlap(struct tds_object* first, struct tds_object* second);
int tds_collision_get_point_overlap(struct tds_object* target, float x, float y);
