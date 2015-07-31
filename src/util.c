#include "util.h"

#include <math.h>

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
