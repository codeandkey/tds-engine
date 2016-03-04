#include "quadtree.h"
#include "memory.h"

#include <stdlib.h>

struct tds_quadtree* tds_quadtree_create(float l, float r, float t, float b) {
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

int tds_quadtree_insert(struct tds_quadtree* ptr, float l, float r, float t, float b, void* data) {
	if (l >= ptr->l && r <= ptr->r && t <= ptr->t && b >= ptr->b) {
		/* Object is at LEAST fully contained in this node. We generate children and see if we can insert into any of them. */

		if (ptr->leaf) {
			ptr->lt = tds_quadtree_create(ptr->l, (ptr->l + ptr->r) / 2.0f, ptr->t, (ptr->t + ptr->b) / 2.0f);
			ptr->lb = tds_quadtree_create(ptr->l, (ptr->l + ptr->r) / 2.0f, (ptr->t + ptr->b) / 2.0f, ptr->b);
			ptr->rt = tds_quadtree_create((ptr->l + ptr->r) / 2.0f, ptr->r, ptr->t, (ptr->t + ptr->b) / 2.0f);
			ptr->rb = tds_quadtree_create((ptr->l + ptr->r) / 2.0f, ptr->r, (ptr->t + ptr->b) / 2.0f, ptr->b);
		}

		if (tds_quadtree_insert(ptr->lt, l, r, t, b, data)) {
			ptr->leaf = 0;
			return 1;
		}

		if (tds_quadtree_insert(ptr->lb, l, r, t, b, data)) {
			ptr->leaf = 0;
			return 1;
		}

		if (tds_quadtree_insert(ptr->rb, l, r, t, b, data)) {
			ptr->leaf = 0;
			return 1;
		}

		if (tds_quadtree_insert(ptr->rt, l, r, t, b, data)) {
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

		return 1;
	} else {
		return 0;
	}
}

void tds_quadtree_walk(struct tds_quadtree* ptr, float l, float r, float t, float b, void* usr, void (*callback)(void*, void*)) {
	if (r < ptr->l || l > ptr->r || t < ptr->b || b > ptr->t) {
		return;
	}

	if (!ptr->leaf) {
		tds_quadtree_walk(ptr->lt, l, r, t, b, usr, callback);
		tds_quadtree_walk(ptr->lb, l, r, t, b, usr, callback);
		tds_quadtree_walk(ptr->rt, l, r, t, b, usr, callback);
		tds_quadtree_walk(ptr->rb, l, r, t, b, usr, callback);
	}

	struct tds_quadtree_entry* cur = ptr->entry_list;

	while (cur) {
		callback(usr, cur->data);
		cur = cur->next;
	}
}

void tds_quadtree_render(struct tds_quadtree* ptr, struct tds_overlay* overlay) {
	if (!ptr) {
		return;
	}

	tds_overlay_set_color(overlay, 1.0f, 1.0f, 1.0f, 1.0f);

	tds_overlay_render_line(overlay, ptr->l, ptr->t, ptr->r, ptr->t, 0.6f, TDS_OVERLAY_WORLDSPACE | TDS_OVERLAY_USE_HIDDENSCALE);
	tds_overlay_render_line(overlay, ptr->l, ptr->t, ptr->l, ptr->b, 0.6f, TDS_OVERLAY_WORLDSPACE | TDS_OVERLAY_USE_HIDDENSCALE);
	tds_overlay_render_line(overlay, ptr->r, ptr->t, ptr->r, ptr->b, 0.6f, TDS_OVERLAY_WORLDSPACE | TDS_OVERLAY_USE_HIDDENSCALE);
	tds_overlay_render_line(overlay, ptr->l, ptr->b, ptr->r, ptr->b, 0.6f, TDS_OVERLAY_WORLDSPACE | TDS_OVERLAY_USE_HIDDENSCALE);

	tds_quadtree_render(ptr->lt, overlay);
	tds_quadtree_render(ptr->rt, overlay);
	tds_quadtree_render(ptr->rb, overlay);
	tds_quadtree_render(ptr->lb, overlay);
}
