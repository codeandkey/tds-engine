#pragma once

#include "object.h"
#include "world.h"

int tds_collision_get_overlap(struct tds_object* first, struct tds_object* second);
int tds_collision_get_point_overlap(struct tds_object* target, float x, float y);

/* tds_collision_get_cast_distance returns the potential distance casted from (xp, yp) to (xf, yf) without intersecting the world. (flags of the terrain returned in flags) */
float tds_collision_get_cast_distance(struct tds_world* world, float xp, float yp, float xf, float yf, float x1, float y1, float x2, float y2, int* flags);

/* tds_collision_get_object_overlap returns the flags of the first collision detected between obj and the world. */
int tds_collision_get_object_overlap(struct tds_world* world, struct tds_object* obj);

/*
 * tds_collision_get_cast_distance is particularly useful in platformers for implementing movement behavior in non-flat worlds.
 * The flags returned can be used to determine different sounds to play depending on terrain.
 */
