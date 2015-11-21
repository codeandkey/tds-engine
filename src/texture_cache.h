#pragma once

#include "texture.h"

struct tds_texture_cache_link {
	struct tds_texture* data;
	struct tds_texture_cache_link* next, *prev;
};

struct tds_texture_cache {
	struct tds_texture_cache_link* head, *tail;
};

struct tds_texture_cache* tds_texture_cache_create(void);
void tds_texture_cache_free(struct tds_texture_cache* ptr);

struct tds_texture* tds_texture_cache_get(struct tds_texture_cache* ptr, const char* texture_name, int tile_x, int tile_y, int wrap_x, int wrap_y);
