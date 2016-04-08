#include "texture_cache.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

struct tds_texture_cache* tds_texture_cache_create(void) {
	struct tds_texture_cache* output = tds_malloc(sizeof(struct tds_texture_cache));

	output->head = NULL;

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

struct tds_texture* tds_texture_cache_get(struct tds_texture_cache* ptr, const char* texture_name, int tile_x, int tile_y, int wrap_x, int wrap_y) {
	struct tds_texture_cache_link* current = ptr->head;

	while (current) {
		if (!strcmp(current->data->filename, texture_name)) {
			tds_logf(TDS_LOG_DEBUG, "Matched [%s] with stored [%s]\n", texture_name, current->data->filename);
			return current->data;
		}

		current = current->next;
	}

	current = tds_malloc(sizeof(struct tds_texture_cache_link));

	current->next = ptr->head;
	ptr->head = current;

	current->data = tds_texture_create(texture_name, tile_x, tile_y);

	if (wrap_x || wrap_y) {
		tds_texture_set_wrap(current->data, wrap_x, wrap_y);
	}

	return current->data;
}
