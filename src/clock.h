#pragma once

#include <sys/time.h>

typedef struct timespec tds_clock_point;

tds_clock_point tds_clock_get_point(void);
double tds_clock_get_ms(tds_clock_point rel);
