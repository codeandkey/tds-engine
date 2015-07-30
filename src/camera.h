#pragma once

#include "display.h"
#include "linmath.h"

struct tds_camera {
	float x, y, z, angle;
	float width, height;

	struct tds_display* disp;
	mat4x4 mat_transform;
};

struct tds_camera* tds_camera_create(struct tds_display* win);
void tds_camera_free(struct tds_camera* ptr);

void tds_camera_set(struct tds_camera* ptr, float camera_size, float x, float y);
