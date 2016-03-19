#include "font_cache.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

struct tds_font_cache* tds_font_cache_create(void) {
	struct tds_font_cache* output = tds_malloc(sizeof(struct tds_font_cache));

	output->head = output->tail = NULL;

	return output;
}

void tds_font_cache_free(struct tds_font_cache* ptr) {
	struct tds_font_cache_link* current = ptr->head, *tmp = NULL;

	while (current) {
		tmp = current;
		tds_font_free(current->data);
		current = current->next;
		tds_free(tmp);
	}

	tds_free(ptr);
}

void tds_font_cache_add(struct tds_font_cache* ptr, const char* font_name, struct tds_font* font) {
	struct tds_font_cache_link* current = NULL;
	current = tds_malloc(sizeof(struct tds_font_cache_link));

	current->prev = ptr->tail;
	current->next = NULL;
	current->name = font_name;
	current->data = font;

	if (!ptr->head) {
		ptr->head = current;
	}

	if (ptr->tail) {
		ptr->tail->next = current;
	}

	ptr->tail = current;
}

struct tds_font* tds_font_cache_get(struct tds_font_cache* ptr, const char* font_name) {
	struct tds_font_cache_link* current = ptr->head;

	while (current) {
		if (!strcmp(current->name, font_name)) {
			return current->data;
		}

		current = current->next;
	}

	tds_logf(TDS_LOG_WARNING, "[%s] not found in font cache\n", font_name);
	return NULL;
}
