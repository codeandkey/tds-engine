#pragma once

#include "texture.h"

struct tds_block_type {
	struct tds_texture* texture;
	int solid;
	int id;
	struct tds_block_type* next;
};

struct tds_block_map {
	struct tds_block_type* block_type_list;
};

struct tds_block_map* tds_block_map_create(void);
void tds_block_map_free(struct tds_block_map* ptr);

void tds_block_map_add(struct tds_block_map* ptr, struct tds_texture* tex, int solid, int id);
struct tds_block_type* tds_block_map_get(struct tds_block_map* ptr, int id);
