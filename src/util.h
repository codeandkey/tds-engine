#pragma once

#include "object.h"

void tds_util_rgb_to_hsv(float r, float g, float b, float* h, float* s, float* v);
void tds_util_hsv_to_rgb(float h, float s, float v, float* r, float* g, float* b);

int tds_util_get_intersect(float x1, float y1, float x2, float y2, struct tds_object* ptr);
float tds_util_angle_distance(float a, float b);
