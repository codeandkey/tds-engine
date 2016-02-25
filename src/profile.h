#pragma once

#include "clock.h"

struct tds_profile_cycle {
	const char* name;
	int mark_count, call_count;
	float time;
	tds_clock_point time_start;
	struct tds_profile_cycle* next;
};

struct tds_profile {
	struct tds_profile_cycle* stack, *list; /* Linked stack structure for cycle tracking. */
};

struct tds_profile* tds_profile_create(void);
void tds_profile_free(struct tds_profile* ptr);

void tds_profile_push(struct tds_profile* ptr, const char* name);
void tds_profile_pop(struct tds_profile* ptr);

void tds_profile_mark(struct tds_profile* ptr);
void tds_profile_flush(struct tds_profile* ptr);
void tds_profile_output(struct tds_profile* ptr);
