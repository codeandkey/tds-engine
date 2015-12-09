#include "collision.h"
#include "log.h"

#include <math.h>

struct _tds_collision_vert {
	float x, y;
};

float _tds_collision_crossz(float x1, float y1, float x2, float y2, float x3, float y3);
int _tds_collision_tri(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

int tds_collision_get_overlap(struct tds_object* first, struct tds_object* second) {
	if (first->x - first->cbox_width / 2.0f > second->x + second->cbox_width / 2.0f) {
		return 0;
	}

	if (first->x + first->cbox_width / 2.0f < second->x - second->cbox_width / 2.0f) {
		return 0;
	}

	if (first->y - first->cbox_height / 2.0f > second->y + second->cbox_height / 2.0f) {
		return 0;
	}

	if (first->y + first->cbox_height / 2.0f > second->y - second->cbox_height / 2.0f) {
		return 0;
	}

	return 1;
}

int tds_collision_get_point_overlap(struct tds_object* ptr, float x, float y) {
	if (x < ptr->x - ptr->cbox_width / 2.0f) {
		return 0;
	}

	if (x > ptr->x + ptr->cbox_width / 2.0f) {
		return 0;
	}

	if (y < ptr->y - ptr->cbox_height / 2.0f) {
		return 0;
	}

	if (y > ptr->y + ptr->cbox_height / 2.0f) {
		return 0;
	}

	return 1;
}
