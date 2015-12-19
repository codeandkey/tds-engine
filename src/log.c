#include "log.h"

#include <stdlib.h>
#include <stdio.h>

static const char* _log_colors[] = {
	"\e[0;31m",
	"\e[0;33m",
	"\e[0;32m",
	"\e[0;34m",
};

static const char* _log_color_reset = "\e[0;39m";

void tds_vlog(int level, const char* fmt, va_list args) {
	if (level <= TDS_LOG_LEVEL) {
		printf(_log_colors[level]);
		vprintf(fmt, args);
		printf(_log_color_reset);
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
