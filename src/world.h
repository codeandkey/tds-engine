#pragma once
#include <stdint.h>

#include "object.h"
#include "vertex_buffer.h"
#include "quadtree.h"

/* each block in the world is strictly 16 units tall, and 16 units wide. */

struct tds_world_hblock {
	int id;
	tds_bcp pos, dim;
	struct tds_world_hblock* next;
	struct tds_vertex_buffer* vb;
};

struct tds_world_segment {
	tds_bcp a, b;
	tds_vec2 n;
	struct tds_world_segment* next, *prev;
};

/* it's a huge waste of.. everything to keep complete block data in memory. */

struct tds_world {
	struct tds_world_hblock* block_list_head, *block_list_tail;
	struct tds_world_segment* segment_list;
	struct tds_vertex_buffer* segment_vb;
	struct tds_quadtree* quadtree;
};

struct tds_world* tds_world_create(void);
void tds_world_free(struct tds_world* ptr);

void tds_world_init(struct tds_world* ptr); /* This creates a blank slate world; tds_world_load will automatically call this if it hasn't been called yet. */
void tds_world_load(struct tds_world* ptr, const uint8_t* block_buffer, int width, int height);
void tds_world_load_hblocks(struct tds_world* ptr, struct tds_world_hlock* block_list_head); /* the linked list is copied, please free it after passing */
void tds_world_save(struct tds_world* ptr, uint8_t* block_buffer, int width, int height);

void tds_world_set_block(struct tds_world* ptr, int x, int y, uint8_t block);
uint8_t tds_world_get_block(struct tds_world* ptr, int x, int y);

int tds_world_get_overlap_fast(struct tds_world* ptr, struct tds_object* obj, float* x, float* y, float* w, float* h, int flag_req, int flag_or, int flag_not); /* The "fast" overlap is a super-quick method of intersection, but it requires that the object is axis-aligned. */
/* tds_world_get_overlap_fast will store the x and y coordinates of the collided hblock in x and y if there is a collision, likewise for cblock width and height in world space */
