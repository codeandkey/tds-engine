#include "coord.h"

#include <math.h>

const tds_bcp tds_bcp_zero = {0};
const tds_vec2 tds_vec2_zero = {0};

tds_bcp tds_bcp_midpoint(tds_bcp a, tds_bcp b) {
	tds_bc tmp;

	if (a.x < b.x) {
		tmp = a.x;
		a.x = b.x;
		b.x = tmp;
	}

	if (a.y < b.y) {
		tmp = a.y;
		a.y = b.y;
		b.y = tmp;
	}

	a.x = (a.x - b.x) / 2 + a.x;
	a.y = (a.y - b.y) / 2 + a.y;

	return a;
}

double tds_bcp_distance(tds_bcp a, tds_bcp b) {
	double x = a.x - b.x, y = a.y - b.y; /* 64-bit precision should be sufficient for values in the 2^32 range */
	return sqrt(pow(x, 2) + pow(y, 2));
}

int tds_bcp_cmpi(tds_bcp a, tds_bc x, tds_bc y) {
	if (a.x < x) return -1;
	if (x > a.x) return 1;
	if (a.y > y) return 1;
	if (y > a.y) return -1;

	return 0;
}

int tds_vec2_cmpi(tds_vec2 a, int32_t x, int32_t y) {
	if (a.x < x) return -1;
	if (x > a.x) return 1;
	if (a.y > y) return 1;
	if (y > a.y) return -1;

	return 0;
}
