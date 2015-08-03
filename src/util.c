#include "util.h"
#include "log.h"

#include <math.h>

static int _tds_util_line_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
static float _tds_util_cross_product(float x1, float y1, float x2, float y2, float x3, float y3);

void tds_util_rgb_to_hsv(float r, float g, float b, float* h, float* s, float* l) {
	double _max = 0.0, _min = 0.0, c = 0.0;

	_max = fmax(r, fmax(g, b));
	_min = fmin(r, fmin(g, b));
	c = _max - _min;
	*h = *s = 0.0f;
	*l = 0.5f * (_max + _min);

	if (c != 0.0) {
		if (_max == r) {
			*h = fmod(((g - b) / c), 6.0);
		} else if (_max == g) {
			*h = ((b - r) / c) + 2.0;
		} else {
			*h = ((r - g) / c) + 4.0;
		}

		*h *= 60.0f;
		*s = c / (1.0f - fabs(2.0f * *l - 1.0));
	}
}

void tds_util_hsv_to_rgb(float h, float s, float l, float* r, float* b, float* g) {
	double c = 0.0f, m = 0.0f, x = 0.0f;
	c = (1.0f - fabs(2 * l - 1.0)) * s;
	m = 1.0 * (l - 0.5 * c);
	x = c * (1.0 - fabs(fmod(h / 60.0, 2) - 1.0));

	if (h >= 0.0f && h < (360.0 / 6.0)) {
		*r = c + m;
		*g = x + m;
		*b = m;
	} else if (h >= (360.0f / 6.0) && h < (360.0 / 3.0)) {
		*r = x + m;
		*g = c + m;
		*b = m;
	} else if (h >= (360.0f / 3.0) && h < (360.0 / 2.0)) {
		*r = m;
		*g = c + m;
		*b = x + m;
	} else if (h >= (360.0f / 2.0) && h < 2.0 * (360.0 / 3.0)) {
		*r = m;
		*g = x + m;
		*b = c + m;
	} else if (h >= 2.0 * (360.0 / 3.0) && h < 5.0 * (360.0 / 6.0)) {
		*r = x + m;
		*g = m;
		*b = c + m;
	} else if (h >= 5.0 * (360.0 / 6.0) && h < 360.0) {
		*r = c + m;
		*g = m;
		*b = x + m;
	} else {
		*r = m;
		*g = m;
		*b = m;
	}
}

int tds_util_get_intersect(float x1, float y1, float x2, float y2, struct tds_object* ptr) {
	float x3, y3, x4, y4;

	for (int i = 0; i < 4; ++i) {
		switch(i) {
		case 0:
			x3 = -ptr->cbox_width / 2.0f + ptr->x;
			y3 = -ptr->cbox_height / 2.0f + ptr->y;
			x4 = ptr->cbox_width / 2.0f + ptr->x;
			y4 = -ptr->cbox_height / 2.0f + ptr->y;
			break;
		case 1:
			x3 = ptr->cbox_width / 2.0f + ptr->x;
			y3 = -ptr->cbox_height / 2.0f + ptr->y;
			x4 = ptr->cbox_width / 2.0f + ptr->x;
			y4 = ptr->cbox_height / 2.0f + ptr->y;
			break;
		case 2:
			x3 = ptr->cbox_width / 2.0f + ptr->x;
			y3 = ptr->cbox_height / 2.0f + ptr->y;
			x4 = -ptr->cbox_width / 2.0f + ptr->x;
			y4 = ptr->cbox_height / 2.0f + ptr->y;
			break;
		case 3:
			x3 = -ptr->cbox_width / 2.0f + ptr->x;
			y3 = ptr->cbox_height / 2.0f + ptr->y;
			x4 = -ptr->cbox_width / 2.0f + ptr->x;
			y4 = -ptr->cbox_height / 2.0f + ptr->y;
			break;
		}

		if (_tds_util_line_intersect(x1, y1, x2, y2, x3, y3, x4, y4)) {
			return 1;
		}
	}

	return 0;
}

int _tds_util_line_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
	/* We check to ensure that each line is split across the projection of the other using 4 cross-product calculations. */

	float cp1 = _tds_util_cross_product(x1, y1, x2, y2, x3, y3);
	float cp2 = _tds_util_cross_product(x1, y1, x2, y2, x4, y4);

	if (((cp1 < 0.0f) == (cp2 < 0.0f)) == 1) {
		return 0;
	} else if (((cp1 > 0.0f) == (cp2 > 0.0f)) == 1) {
		return 0;
	} else {
		/* cp1 and cp2 are both 0 */
		/* AKA parallel segments occupying the same line */
		/* not sure how to handle that effectively. */
	}

	float cp3 = _tds_util_cross_product(x3, y3, x4, y4, x1, y1);
	float cp4 = _tds_util_cross_product(x3, y3, x4, y4, x2, y2);

	if (((cp3 < 0.0f) == (cp4 < 0.0f)) == 1) {
		return 0;
	} else if (((cp3 > 0.0f) == (cp4 > 0.0f)) == 1) {
		return 0;
	} else {
		/* cp3 and cp4 are both 0 */
		/* AKA parallel segments occupying the same line */
		/* not sure how to handle that effectively. */
	}

	return 1;
}

float _tds_util_cross_product(float x1, float y1, float x2, float y2, float x3, float y3) {
	/* Cross product : (y2x1) - (x2y1) */

	float _x1 = x2 - x1;
	float _y1 = y2 - y1;
	float _x2 = x3 - x1;
	float _y2 = y3 - y1;

	return (_y2 * _x1) - (_x2 * _y1);
}

float tds_util_get_angle(float y, float x) {
	float output = atan2f(y, x);

	if (output < 0.0f) {
		output += 3.141f * 2.0f;
	}

	return output;
}

float tds_util_angle_distance(float a, float b) {
	float na = fmod(a, 2.0f * M_PI), nb = fmod(b, 2.0f * M_PI);

	if (na < 0.0f) {
		na += 2.0f * M_PI;
	}

	if (nb < 0.0f) {
		nb += 2.0f * M_PI;
	}

	/* Both na and nb should now be in the range [0, 2pi) */

	return fmin(fabs(na - nb), (2.0f * M_PI) - fabs(na - nb));
}
