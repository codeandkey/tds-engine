#include "texture_cache.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

struct tds_texture_cache* tds_texture_cache_create(void) {
	struct tds_texture_cache* output = tds_malloc(sizeof(struct tds_texture_cache));

	output->head = output->tail = NULL;

	return output;
}

void tds_texture_cache_free(struct tds_texture_cache* ptr) {
	struct tds_texture_cache_link* current = ptr->head, *tmp = NULL;

	while (current) {
		tds_texture_free(current->data);
		tmp = current;
		current = current->next;
		tds_free(tmp);
	}

	tds_free(ptr);
}

struct tds_texture* tds_texture_cache_get(struct tds_texture_cache* ptr, const char* texture_name) {
	struct tds_texture_cache_link* current = ptr->head;

	while (current) {
		if (!strcmp(current->data->filename, texture_name)) {
			return current->data;
		}

		current = current->next;
	}

	tds_logf(TDS_LOG_DEBUG, "%s not found in texture cache, loading..", texture_name);
	current = tds_malloc(sizeof(struct tds_texture_cache_link));

	current->prev = ptr->tail;
	current->next = NULL;

	if (!ptr->head) {
		ptr->head = current;
	}

	if (!ptr->tail) {
		ptr->tail = current;
	} else {
		ptr->tail->next = current;
	}

	current->data = tds_texture_create(texture_name);
	return current->data;
}
