#pragma once

#include "sprite.h"

struct tds_sprite_cache_link {
	struct tds_sprite* data;
	struct tds_sprite_cache_link* next;

	const char* name;
};

struct tds_sprite_cache {
	struct tds_sprite_cache_link* head;
};

struct tds_sprite_cache* tds_sprite_cache_create(void);
void tds_sprite_cache_free(struct tds_sprite_cache* ptr);

void tds_sprite_cache_add(struct tds_sprite_cache* ptr, const char* sprite_name, struct tds_sprite* obj);
struct tds_sprite* tds_sprite_cache_get(struct tds_sprite_cache* ptr, const char* sprite_name);
