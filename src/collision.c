#include "collision.h"
#include "log.h"

#include <math.h>

struct _tds_collision_vert {
	float x, y;
};

float _tds_collision_crossz(float x1, float y1, float x2, float y2, float x3, float y3);
int _tds_collision_tri(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

int tds_collision_get_overlap(struct tds_object* first, struct tds_object* second) {
	float first_r = sqrtf(pow(first->cbox_height / 2.0f, 2.0f) + pow(first->cbox_width / 2.0f, 2.0f));
	float second_r = sqrtf(pow(second->cbox_height / 2.0f, 2.0f) + pow(second->cbox_height / 2.0f, 2.0f));

	/* first_r and second_r are the virtual radii of the two objects;
	 * if the dist. between the centers is more than f_r + s_r there is no way they will collide */

	float center_dist = sqrtf(pow(first->x - second->x, 2.0f) + pow(first->y - second->y, 2.0f));

	if (center_dist > first_r + second_r) {
		return 0;
	}

	if (first->x - first_r > second->x + second_r) {
		return 0;
	}

	if (first->x + first_r < second->x - second_r) {
		return 0;
	}

	if (first->y + first_r < second->y - second_r) {
		return 0;
	}

	if (first->y - first_r > second->y + second_r) {
		return 0;
	}

	return 1;
}

int tds_collision_get_point_overlap(struct tds_object* ptr, float x, float y) {
	float diagonal_angle = atan2f(ptr->cbox_height, ptr->cbox_width);
	float diagonal_length = sqrtf(pow(ptr->cbox_width, 2) + pow(ptr->cbox_height, 2));

	float tlx = cos(M_PI - diagonal_angle) * diagonal_length + ptr->x;
	float tly = sin(M_PI - diagonal_angle) * diagonal_length + ptr->y;
	float blx = cos(M_PI + diagonal_angle) * diagonal_length + ptr->x;
	float bly = sin(M_PI + diagonal_angle) * diagonal_length + ptr->y;
	float trx = cos(diagonal_angle) * diagonal_length + ptr->x;
	float try = sin(diagonal_angle) * diagonal_length + ptr->y;
	float brx = cos(-diagonal_angle) * diagonal_length + ptr->x;
	float bry = sin(-diagonal_angle) * diagonal_length + ptr->y;

	int c1 = _tds_collision_tri(brx, bry, trx, try, tlx, tly, x, y);

	if (c1) {
		return 1;
	}

	int c2 = _tds_collision_tri(brx, bry, tlx, tly, blx, bly, x, y);

	return c2;
}

float _tds_collision_crossz(float x1, float y1, float x2, float y2, float x3, float y3) {
	return ((x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1));
}

int _tds_collision_tri(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
	float pol = 0.0f;
	pol = _tds_collision_crossz(x1, y1, x2, y2, x4, y4);

	if ((_tds_collision_crossz(x2, y2, x3, y3, x4, y4) > 0.0f) != (pol > 0.0f)) {
		return 0;
	}

	if ((_tds_collision_crossz(x3, y3, x1, y1, x4, y4) > 0.0f) != (pol > 0.0f)) {
		return 0;
	}

	return 1;
}
