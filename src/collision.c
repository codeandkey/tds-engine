#include "collision.h"

#include <math.h>

struct _tds_collision_vert {
	float x, y;
};

int tds_collision_get_overlap(struct tds_object* first, struct tds_object* second) {
	/* The current collision system will disregard all angles and proceed only with the object dimensions. */
	/* If the game really does need rotated rect. collisions (MUCH more complex than AABB, we can deal with that later. */
	/* There is an optimizing pass written here but it really only helps the performance of more complicated algorithms.. */
	/* AABB is much faster than the optimized pass :D */

	if (first->x + first->cbox_width / 2.0f < second->x - second->cbox_width / 2.0f || first->x - first->cbox_width / 2.0f > second->x + second->cbox_width / 2.0f) {
		return 0;
	}

	if (first->y + first->cbox_height / 2.0f < second->y - second->cbox_height / 2.0f || first->y - first->cbox_height / 2.0f > second->y + second->cbox_height / 2.0f) {
		return 0;
	}

	return 1;

	/* This only checks object collisions, it disregards everything about the sprites. */
	/* First pass - basic distance check. */

	float first_r = sqrtf(pow(first->cbox_height / 2.0f, 2.0f) + pow(first->cbox_width / 2.0f, 2.0f));
	float second_r = sqrtf(pow(second->cbox_height / 2.0f, 2.0f) + pow(second->cbox_height / 2.0f, 2.0f));

	/* first_r and second_r are the virtual radii of the two objects;
	 * if the dist. between the centers is more than f_r + s_r there is no way they will collide */

	float center_dist = sqrtf(pow(first->x - second->x, 2.0f) + pow(first->y - second->y, 2.0f));

	if (center_dist < first_r + second_r) {
		return 0;
	}

	/* At this point, we know the rectangles are close enough for a potential collision, we do a more precise test */
}
