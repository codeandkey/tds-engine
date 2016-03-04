#pragma once
#include "overlay.h"

struct tds_quadtree_entry {
	void* data;
	struct tds_quadtree_entry* next;
};

struct tds_quadtree {
	struct tds_quadtree* lb, *lt, *rb, *rt;
	float l, r, t, b;
	int leaf;
	struct tds_quadtree_entry* entry_list;
};

struct tds_quadtree* tds_quadtree_create(float l, float r, float t, float b);
void tds_quadtree_free(struct tds_quadtree* ptr);

/* quadtree_free will free the quadtree node AND all of the child nodes.
 * it will NOT free the data which has been passed to it. */

int tds_quadtree_insert(struct tds_quadtree* ptr, float l, float r, float t, float b, void* data); /* Returns 1 if successfully inserted into node or any children */
void tds_quadtree_walk(struct tds_quadtree* ptr, float l, float r, float t, float b, void* usr, void (*user_callback)(void*, void*));

void tds_quadtree_render(struct tds_quadtree* ptr, struct tds_overlay* overlay);

/* user_callback is called with (usr, <quadtree entry data>) */
