#include "sound_cache.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

struct tds_sound_cache* tds_sound_cache_create(void) {
	struct tds_sound_cache* output = tds_malloc(sizeof(struct tds_sound_cache));

	output->head = NULL;

	return output;
}

void tds_sound_cache_free(struct tds_sound_cache* ptr) {
	struct tds_sound_cache_link* current = ptr->head, *tmp = NULL;

	while (current) {
		tds_sound_buffer_free(current->data);
		tmp = current;
		current = current->next;
		tds_free(tmp);
	}

	tds_free(ptr);
}

void tds_sound_cache_add(struct tds_sound_cache* ptr, const char* sound_name, struct tds_sound_buffer* sound) {
	struct tds_sound_cache_link* current = NULL;
	current = tds_malloc(sizeof(struct tds_sound_cache_link));

	current->next = ptr->head;
	current->name = sound_name;
	current->data = sound;

	ptr->head = current;
}

struct tds_sound_buffer* tds_sound_cache_get(struct tds_sound_cache* ptr, const char* sound_name) {
	struct tds_sound_cache_link* current = ptr->head;

	while (current) {
		if (!strcmp(current->name, sound_name)) {
			return current->data;
		}

		current = current->next;
	}

	tds_logf(TDS_LOG_CRITICAL, "%s not found in sound cache\n", sound_name);
	return current->data;
}
