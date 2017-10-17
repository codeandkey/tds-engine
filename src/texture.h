#pragma once

#include "coord.h"

struct tds_texture_frame {
	float left, right, bottom, top;
};

struct tds_texture {
	char* filename;
	unsigned int gl_id;
	struct tds_texture_frame* frame_list;
	unsigned int frame_count;
	tds_bcp dim;
};

struct tds_texture* tds_texture_create(const char* filename, int tile_x, int tile_y); /* Use tds_texture_cache_get for game purposes. */
void tds_texture_set_wrap(struct tds_texture* ptr, int wrap_x, int wrap_y);
void tds_texture_free(struct tds_texture* ptr);
