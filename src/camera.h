#pragma once

#include "display.h"
#include "linmath.h"
#include "coord.h"

/*
 * the camera operates within it's own sized space (x, 1.0f) in dim
 * however we need to be operating with larger coordinates and don't want floating-point
 * precision to fuck anything up which it will at high coordinates -- so, we maintain the camera position in game space
 * and then compute relative distances to keep values small.
 */ 

struct tds_camera {
	float angle;
	tds_bcp pos, dim; /* represents the CENTER of the camera for convienience. */
	float hidden_scale; /* Changes the camera size without altering the width and height values. */

	mat4x4 mat_transform;
};

struct tds_camera* tds_camera_create(struct tds_display* win);
void tds_camera_free(struct tds_camera* ptr);

void tds_camera_set(struct tds_camera* ptr, tds_bcp center_pos, tds_bc height);
void tds_camera_set_raw(struct tds_camera* ptr, tds_bcp pos, tds_bcp dim);
