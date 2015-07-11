#pragma once

struct tds_camera {
	float x, y, z, angle;
	float width, height;
};

struct tds_camera* tds_camera_create(void);
void tds_camera_free(struct tds_camera* ptr);
