#include "quadtree.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>

struct tds_quadtree* tds_quadtree_create(tds_bc l, tds_bc r, tds_bc t, tds_bc b) {
	struct tds_quadtree* output = tds_malloc(sizeof *output);

	output->l = l;
	output->r = r;
	output->t = t;
	output->b = b;

	output->entry_list = NULL;
	output->lb = output->lt = output->rb = output->rt = NULL;
	output->leaf = 1;

	return output;
}

void tds_quadtree_free(struct tds_quadtree* ptr) {
	if (!ptr) {
		return; /* This helps shorten the code .. a ton. */
	}

	tds_quadtree_free(ptr->lb);
	tds_quadtree_free(ptr->lt);
	tds_quadtree_free(ptr->rb);
	tds_quadtree_free(ptr->rt);

	struct tds_quadtree_entry* cur = ptr->entry_list, *tmp = NULL;

	while (cur) {
		tmp = cur->next;
		tds_free(cur);
		cur = tmp;
	}

	tds_free(ptr);
}

int tds_quadtree_insert(struct tds_quadtree* ptr, tds_bcp pos, tds_bcp dim, void* data) {
	tds_bcp tr = pos; /* tr exclusive, so we decrement the dim */
	tr.x += dim.x - 1;
	tr.y += dim.y - 1;

	if (pos.x >= ptr->l && tr.x <= ptr->r && tr.y <= ptr->t && pos.y >= ptr->b) {
		/* Object is at LEAST fully contained in this node. We generate children and see if we can insert into any of them. */

		tds_bcp c; /* construct 4 corners and center point */

		c.x = ptr->l + (ptr->r - ptr->l) / 2; /* since tr, br are inclusive this means that c will be on the lower left of the four centerpieces */
		c.y = ptr->b + (ptr->t - ptr->b) / 2;

		/* since we are doing this on a pixel basis now, we can evenly split the coordinate system so the lines are directly adjacent! */

		if (ptr->leaf) {
			ptr->lt = tds_quadtree_create(ptr->l, c.x, ptr->t, c.y + 1);
			ptr->lb = tds_quadtree_create(ptr->l, c.x, c.y, ptr->b);
			ptr->rt = tds_quadtree_create(c.x + 1, ptr->r, ptr->t, c.y + 1);
			ptr->rb = tds_quadtree_create(c.x + 1, ptr->r, c.y, ptr->b);
		}

		if (tds_quadtree_insert(ptr->lt, pos, dim, data)) {
			ptr->leaf = 0;
			return 1;
		}

		if (tds_quadtree_insert(ptr->lb, pos, dim, data)) {
			ptr->leaf = 0;
			return 1;
		}

		if (tds_quadtree_insert(ptr->rb, pos, dim, data)) {
			ptr->leaf = 0;
			return 1;
		}

		if (tds_quadtree_insert(ptr->rt, pos, dim, data)) {
			ptr->leaf = 0;
			return 1;
		}

		/* The object couldn't be inserted into any children, but it does fit within the node, we add it to our entries. */
		struct tds_quadtree_entry* next = tds_malloc(sizeof *next);
		next->data = data;
		next->next = ptr->entry_list;
		ptr->entry_list = next;

		/* We can also get rid of our children. Kill the children! */
		if (ptr->leaf) {
			tds_quadtree_free(ptr->lt);
			tds_quadtree_free(ptr->rt);
			tds_quadtree_free(ptr->lb);
			tds_quadtree_free(ptr->rb);

			ptr->lt = ptr->lb = ptr->rt = ptr->rb = NULL;
		}

		tds_logf(TDS_LOG_DEBUG, "inserted %p (pos %u,%u dim %u,%u) into quadtree leaf %p\n", data, pos.x, pos.y, dim.x, dim.y, ptr);

		return 1;
	} else {
		return 0;
	}
}

void tds_quadtree_walk(struct tds_quadtree* ptr, tds_bcp pos, tds_bcp dim, void* usr, void (*callback)(void*, void*)) {
	tds_bcp tr = pos; /* exclusive tr box on the right side, so we modify it slightly */
	tr.x += dim.x - 1;
	tr.y += dim.y - 1;

	if (tr.x < ptr->l || pos.x > ptr->r || tr.y < ptr->b || pos.y > ptr->t) {
		return;
	}

	if (!ptr->leaf) {
		tds_quadtree_walk(ptr->lt, pos, dim, usr, callback);
		tds_quadtree_walk(ptr->lb, pos, dim, usr, callback);
		tds_quadtree_walk(ptr->rt, pos, dim, usr, callback);
		tds_quadtree_walk(ptr->rb, pos, dim, usr, callback);
	}

	struct tds_quadtree_entry* cur = ptr->entry_list;

	while (cur) {
		callback(usr, cur->data);
		cur = cur->next;
	}
}
