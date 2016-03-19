#pragma once

#include "display.h"
#include "linmath.h"

struct tds_camera {
	float x, y, z, angle;
	float width, height;
	float hidden_scale; /* Changes the camera size without altering the width and height values. */

	mat4x4 mat_transform;
};

struct tds_camera* tds_camera_create(struct tds_display* win);
void tds_camera_free(struct tds_camera* ptr);

void tds_camera_set(struct tds_camera* ptr, float camera_size, float x, float y);
void tds_camera_set_raw(struct tds_camera* ptr, float width, float height, float x, float y);
