#include "log.h"

#include <stdlib.h>
#include <stdio.h>

void tds_vlog(int level, const char* fmt, va_list args) {
	if (level <= TDS_LOG_LEVEL) {
		vprintf(fmt, args);
	}

	if (level == TDS_LOG_CRITICAL) {
		printf("TDS: Exiting due to critical failure.\n");
		exit(-1);
	}
}

void tds_log(int level, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	tds_vlog(level, fmt, args);

	va_end(args);
}
