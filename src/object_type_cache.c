#include "object_type_cache.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

struct tds_object_type_cache* tds_object_type_cache_create(void) {
	struct tds_object_type_cache* output = tds_malloc(sizeof(struct tds_object_type_cache));

	output->head = output->tail = NULL;

	return output;
}

void tds_object_type_cache_free(struct tds_object_type_cache* ptr) {
	struct tds_object_type_cache_link* current = ptr->head, *tmp = NULL;

	while (current) {
		tmp = current;
		current = current->next;
		tds_free(tmp);
	}

	tds_free(ptr);
}

void tds_object_type_cache_add(struct tds_object_type_cache* ptr, const char* object_type_name, struct tds_object_type* object_type) {
	struct tds_object_type_cache_link* current = NULL;
	current = tds_malloc(sizeof(struct tds_object_type_cache_link));

	current->prev = ptr->tail;
	current->next = NULL;
	current->name = object_type_name;
	current->data = object_type;

	if (!ptr->head) {
		ptr->head = current;
	}

	if (ptr->tail) {
		ptr->tail->next = current;
	}

	ptr->tail = current;
}

struct tds_object_type* tds_object_type_cache_get(struct tds_object_type_cache* ptr, const char* object_type_name) {
	struct tds_object_type_cache_link* current = ptr->head;

	while (current) {
		if (!strcmp(current->name, object_type_name)) {
			return current->data;
		}

		current = current->next;
	}

	tds_logf(TDS_LOG_CRITICAL, "%s not found in object_type cache\n", object_type_name);
	return current->data;
}
