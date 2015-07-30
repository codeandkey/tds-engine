#include "clock.h"
#include "log.h"

#include <time.h>

tds_clock_point tds_clock_get_point(void) {
	tds_clock_point output;
	clock_gettime(CLOCK_MONOTONIC, &output);
	return output;
}

double tds_clock_get_ms(tds_clock_point rel) {
	tds_clock_point cur = tds_clock_get_point();

	time_t dif_sec = cur.tv_sec - rel.tv_sec;
	long dif_nsec = cur.tv_nsec - rel.tv_nsec;

	double ms = 1000.0 * (double) dif_sec + (double) dif_nsec / 1000000.0;

	return ms;
}
