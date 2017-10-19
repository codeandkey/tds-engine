#pragma once

#include "coord.h"

struct tds_quadtree_entry {
	void* data;
	struct tds_quadtree_entry* next;
};

struct tds_quadtree {
	struct tds_quadtree* lb, *lt, *rb, *rt;
	tds_bc l, r, t, b; /* INCLUSIVE coordinates for the bounding box */
	int leaf;
	struct tds_quadtree_entry* entry_list;
};

struct tds_quadtree* tds_quadtree_create(tds_bc l, tds_bc r, tds_bc t, tds_bc b);
void tds_quadtree_free(struct tds_quadtree* ptr);

/* quadtree_free will free the quadtree node AND all of the child nodes.
 * it will NOT free the data which has been passed to it. */

int tds_quadtree_insert(struct tds_quadtree* ptr, tds_bcp pos, tds_bcp dim, void* data); /* Returns 1 if successfully inserted into node or any children */
void tds_quadtree_walk(struct tds_quadtree* ptr, tds_bcp pos, tds_bcp dim, void* usr, void (*user_callback)(void*, void*));

/* user_callback is called with (usr, <quadtree entry data>) */
