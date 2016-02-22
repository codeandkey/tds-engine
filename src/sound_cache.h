#pragma once

#include "sound_buffer.h"

struct tds_sound_cache_link {
	struct tds_sound_buffer* data;
	struct tds_sound_cache_link* next;

	const char* name;
};

struct tds_sound_cache {
	struct tds_sound_cache_link* head;
};

struct tds_sound_cache* tds_sound_cache_create(void);
void tds_sound_cache_free(struct tds_sound_cache* ptr);

void tds_sound_cache_add(struct tds_sound_cache* ptr, const char* sound_name, struct tds_sound_buffer* obj);
struct tds_sound_buffer* tds_sound_cache_get(struct tds_sound_cache* ptr, const char* sound_name);
