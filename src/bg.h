#pragma once

#include "texture.h"

/*
 * backgrounds are stored in linked lists, each corresponding to a layer..
 * to ease sorting and simplify rendering, each list will reside in a fixed slot.
 */

#define TDS_BG_LAYERS 16

struct tds_bg_entry {
	struct tds_texture* tex;
	struct tds_bg_entry* next;
	int factor_y, factor_x;
};

struct tds_bg {
	struct tds_bg_entry* layers[TDS_BG_LAYERS];
};

struct tds_bg* tds_bg_create(void);
void tds_bg_free(struct tds_bg* ptr);

void tds_bg_push(struct tds_bg* ptr, int layer, struct tds_texture* tex, int factor_x, int factor_y);

void tds_bg_flush(struct tds_bg* ptr);
void tds_bg_flush_layer(struct tds_bg* ptr, int layer);
