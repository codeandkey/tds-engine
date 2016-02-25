#include "profile.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

struct tds_profile* tds_profile_create(void) {
	struct tds_profile* output = tds_malloc(sizeof *output);

	output->stack = output->list = NULL;
	return output;
}

void tds_profile_free(struct tds_profile* ptr) {
	tds_profile_flush(ptr);
	tds_free(ptr);
}

void tds_profile_push(struct tds_profile* ptr, const char* name) {
	struct tds_profile_cycle* new_cycle = tds_malloc(sizeof *new_cycle);

	/*
	 * Not really a stack -- more like an N-child expanding tree.
	 * The decision to add as a parallel or child node depends on the current location in the stack.
	 */

	new_cycle->name = name;
	new_cycle->mark_count = 0;
	new_cycle->time_start = tds_clock_get_point();
	new_cycle->time = 0.0f;
	new_cycle->next = ptr->stack;

	ptr->stack = new_cycle;
}

void tds_profile_pop(struct tds_profile* ptr) {
	if (!ptr->stack) {
		return;
	}

	ptr->stack->time = tds_clock_get_ms(ptr->stack->time_start);

	struct tds_profile_cycle* cur = ptr->list, *tmp = NULL;

	while (cur) {
		if (!strcmp(cur->name, ptr->stack->name)) {
			cur->time += ptr->stack->time;
			cur->mark_count += ptr->stack->mark_count;
			tmp = ptr->stack->next;
			tds_free(ptr->stack);
			ptr->stack = tmp;
			return;
		}

		cur = cur->next;
	}

	tmp = ptr->stack->next;
	ptr->stack->next = ptr->list;
	ptr->list = ptr->stack;
	ptr->stack = tmp;
}

void tds_profile_mark(struct tds_profile* ptr) {
	if (!ptr->stack) {
		return;
	}

	ptr->stack->mark_count++;
}

void tds_profile_flush(struct tds_profile* ptr) {
	while (ptr->stack) {
		tds_profile_pop(ptr);
	}

	struct tds_profile_cycle* tmp = NULL;

	while (ptr->list) {
		tmp = ptr->list->next;
		tds_free(ptr->list);
		ptr->list = tmp;
	}
}

void tds_profile_output(struct tds_profile* ptr) {
	struct tds_profile_cycle* cur = ptr->list;
	tds_logf(TDS_LOG_MESSAGE, "-- Profile output statistics --\n");

	while (cur) {
		tds_logf(TDS_LOG_MESSAGE, "%-20s | %-10f ms | %-20d\n", cur->name, cur->time, cur->mark_count);

		cur = cur->next;
	}

	tds_logf(TDS_LOG_MESSAGE, "-- End profile output statistics --\n");
}
