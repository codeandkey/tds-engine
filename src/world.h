#pragma once
#include <stdint.h>

#include "object.h"
#include "vertex_buffer.h"
#include "quadtree.h"

#define TDS_WORLD_BLOCK_SIZE 0.5f

struct tds_world_hblock {
	int x, y, w, id;
	struct tds_world_hblock* next;
	struct tds_vertex_buffer* vb;
};

struct tds_world_segment {
	float x1, y1, x2, y2, nx, ny; /* Segment locations are not in world space, they are in game space. No conversion necessary. */
	struct tds_world_segment* next, *prev;
};

struct tds_world {
	int** buffer, width, height;
	struct tds_world_hblock* block_list_head, *block_list_tail;
	struct tds_world_segment* segment_list;
	struct tds_vertex_buffer* segment_vb;
	struct tds_quadtree* quadtree;
};

struct tds_world* tds_world_create(void);
void tds_world_free(struct tds_world* ptr);

void tds_world_init(struct tds_world* ptr, int width, int height); /* This creates a blank slate world; tds_world_load will automatically call this if it hasn't been called yet. */
void tds_world_load(struct tds_world* ptr, const uint8_t* block_buffer, int width, int height);
void tds_world_save(struct tds_world* ptr, uint8_t* block_buffer, int width, int height);

void tds_world_set_block(struct tds_world* ptr, int x, int y, uint8_t block);
uint8_t tds_world_get_block(struct tds_world* ptr, int x, int y);

int tds_world_get_overlap_fast(struct tds_world* ptr, struct tds_object* obj, float* x, float* y, float* w, float* h, int flag_req, int flag_or, int flag_not); /* The "fast" overlap is a super-quick method of intersection, but it requires that the object is axis-aligned. */
/* tds_world_get_overlap_fast will store the x and y coordinates of the collided hblock in x and y if there is a collision, likewise for cblock width and height in world space */
