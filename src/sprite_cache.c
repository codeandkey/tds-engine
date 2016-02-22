#include "sprite_cache.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

struct tds_sprite_cache* tds_sprite_cache_create(void) {
	struct tds_sprite_cache* output = tds_malloc(sizeof(struct tds_sprite_cache));

	output->head = NULL;

	return output;
}

void tds_sprite_cache_free(struct tds_sprite_cache* ptr) {
	struct tds_sprite_cache_link* current = ptr->head, *tmp = NULL;

	while (current) {
		tds_sprite_free(current->data);
		tmp = current;
		current = current->next;
		tds_free(tmp);
	}

	tds_free(ptr);
}

void tds_sprite_cache_add(struct tds_sprite_cache* ptr, const char* sprite_name, struct tds_sprite* sprite) {
	struct tds_sprite_cache_link* current = NULL;
	current = tds_malloc(sizeof(struct tds_sprite_cache_link));

	current->next = ptr->head;
	current->name = sprite_name;
	current->data = sprite;

	ptr->head = current;
}

struct tds_sprite* tds_sprite_cache_get(struct tds_sprite_cache* ptr, const char* sprite_name) {
	struct tds_sprite_cache_link* current = ptr->head;

	while (current) {
		if (!strcmp(current->name, sprite_name)) {
			return current->data;
		}

		current = current->next;
	}

	tds_logf(TDS_LOG_CRITICAL, "%s not found in sprite cache\n", sprite_name);
	return current->data;
}
