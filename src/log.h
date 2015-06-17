#pragma once

#include <stdarg.h>

#define TDS_LOG_CRITICAL 0
#define TDS_LOG_WARNING  1
#define TDS_LOG_MESSAGE  2
#define TDS_LOG_DEBUG  3

#define TDS_LOG_LEVEL TDS_LOG_DEBUG

void tds_vlog(int level, const char* fmt, va_list args);
void tds_log(int level, const char* fmt, ...);

#define tds_logf(lvl, fmt, ...) { tds_log(lvl, "[%s:%d] " fmt, __func__, lvl, ##__VA_ARGS__); }
