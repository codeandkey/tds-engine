#include "collision.h"
#include "log.h"

#include <math.h>

int tds_collision_get_overlap(struct tds_object* first, struct tds_object* second) {
	if (first->pos.x + first->cbox.x < second->pos.x) return 0;
	if (second->pos.x + second->cbox.x < first->pos.x) return 0;
	if (first->pos.y + first->cbox.y < second->pos.y) return 0;
	if (second->pos.y + second->cbox.y < first->pos.y) return 0;

	return 1;
}

int tds_collision_get_point_overlap(struct tds_object* ptr, tds_bcp pt) {
	if (pt.x < ptr->pos.x) return 0;
	if (pt.x > ptr->pos.x + ptr->cbox.x) return 0; /* consider changing this to GE */
	if (pt.y < ptr->pos.y) return 0;
	if (pt.y > ptr->pos.y + ptr->cbox.y) return 0;

	return 1;
}
