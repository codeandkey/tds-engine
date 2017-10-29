#include "world.h"
#include "log.h"
#include "memory.h"
#include "engine.h"

#include <GLXW/glxw.h>
#include <string.h>

/*
 * UPDATE: originally we were going to stream world data into an hblock generator in tds_loader, however..
 * block data is decoded from base64 all at once anyway -- it's nicer to pass it to tds_world for hblock generation anyway.
 * RAM isn't a worry as we're not loading a fraction of the entire i32^2 space, as we pass a chunk with dimension and offset.
 * so, we can still do block generation here and keep everything pretty.
 */

static void _tds_world_generate_quadtree(struct tds_world* ptr);
static void _tds_world_generate_segments(struct tds_world* ptr);
static void _tds_world_deinit(struct tds_world* ptr);

struct tds_world* tds_world_create(void) {
	struct tds_world* output = tds_malloc(sizeof *output);

	output->block_list_head = output->block_list_tail = 0;
	output->quadtree = NULL;

	return output;
}

void _tds_world_deinit(struct tds_world* ptr) {
	if (ptr->block_list_head) {
		struct tds_world_hblock* cur = ptr->block_list_head, *tmp = 0;

		while (cur) {
			tmp = cur->next;
			tds_vertex_buffer_free(cur->vb);
			tds_free(cur);
			cur = tmp;
		}
	}

	struct tds_world_segment* head = ptr->segment_list, *cur = NULL;

	while (head) {
		cur = head->next;
		tds_free(head);
		head = cur;
	}

	if (ptr->segment_vb) {
		tds_vertex_buffer_free(ptr->segment_vb);
	}

	if (ptr->quadtree) {
		tds_quadtree_free(ptr->quadtree);
	}
}

void tds_world_free(struct tds_world* ptr) {
	_tds_world_deinit(ptr);
	tds_free(ptr);

	tds_logf(TDS_LOG_DEBUG, "Freed world at %p\n", ptr);
}

void tds_world_init(struct tds_world* ptr) {
	_tds_world_deinit(ptr);
	memset(ptr, 0, sizeof *ptr);
}

int tds_world_get_overlap_fast(struct tds_world* ptr, struct tds_object* obj, int flag_req, int flag_or, int flag_not) {
	/* Another important function. Intersection testing with axis-aligned objects. */

	tds_bcp pos = obj->pos, tr = pos;
	tr.x += obj->cbox.x;
	tr.y += obj->cbox.y;

	/* The world block coordinates will be treated as centers. The block at [0, 0] will be centered on the origin. */
	struct tds_world_hblock* cblock = ptr->block_list_head;

	while (cblock) {
		tds_bcp c_pos = cblock->pos, c_tr = c_pos;
		c_tr.x += cblock->dim.x;
		c_tr.y += cblock->dim.y;

		if (pos.x > c_tr.x) {
			cblock = cblock->next;
			continue;
		}

		if (tr.x < c_pos.x) {
			cblock = cblock->next;
			continue;
		}

		if (tr.y < c_pos.y) {
			cblock = cblock->next;
			continue;
		}

		if (pos.y > c_tr.y) {
			cblock = cblock->next;
			continue;
		}

		int flags = tds_block_map_get(tds_engine_global->block_map_handle, cblock->id).flags;

		if ((flags & flag_req) != flag_req) {
			cblock = cblock->next;
			continue;
		}

		if (!(flags & flag_or)) {
			cblock = cblock->next;
			continue;
		}

		if (flags & flag_not) {
			cblock = cblock->next;
			continue;
		}

		return flags;
	}

	return 0;
}

void _tds_world_generate_segments(struct tds_world* ptr) {
	struct tds_world_segment* head = ptr->segment_list, *cur = NULL;
	struct tds_world_hblock* cur_block = ptr->block_list_head;

	while (head) {
		cur = head->next;
		tds_free(head);
		head = cur;
	}

	if (ptr->segment_vb) {
		tds_vertex_buffer_free(ptr->segment_vb);
	}

	ptr->segment_list = NULL;
	ptr->segment_vb = NULL;

	tds_logf(TDS_LOG_DEBUG, "Starting redundant segment generation phase.\n");

	while (cur_block) {
		/* we can't add segments solely based on hblocks as they can tend to "overlap" in strange ways which can affect the lighting engine */
		int flags = tds_block_map_get(tds_engine_global->block_map_handle, cur_block->id).flags;

		if (flags & TDS_BLOCK_TYPE_NOLIGHT || !(flags & TDS_BLOCK_TYPE_SOLID)) {
			cur_block = cur_block->next;
			continue;
		}

		/* todo: to accommdate the world no longer having a buffer we can't place segments just on outfacing blocks -- we proceed to place them around every block which should cast a light and match the shape.
		 * it would certainly be nicer to the lighting engine to restrict segments to actual edges. */

		for (int i = 0; i < cur_block->dim.x; ++i) {
			tds_bcp block_bl = cur_block->pos;
			tds_bcp block_tr = cur_block->pos;

			block_tr.x += cur_block->dim.x - 1; /* we consider the top-right corner of the block to be on the interior of the block */
			block_tr.y += cur_block->dim.y - 1;

			if (!(flags & (TDS_BLOCK_TYPE_RTSLOPE | TDS_BLOCK_TYPE_RBSLOPE))) {
				/* Out-facing right segment. */
				cur = tds_malloc(sizeof *cur);
				cur->a.x = block_tr.x;
				cur->a.y = block_bl.y;
				cur->b.x = block_tr.x;
				cur->b.y = block_tr.y;
				cur->n.x = 1;
				cur->n.y = 0;
				cur->next = ptr->segment_list;
				if (ptr->segment_list) {
					ptr->segment_list->prev = cur;
				}
				ptr->segment_list = cur;
			}

			if (!(flags & (TDS_BLOCK_TYPE_LTSLOPE | TDS_BLOCK_TYPE_LBSLOPE))) {
				/* Out-facing left segment. */
				cur = tds_malloc(sizeof *cur);
				cur->a.x = block_bl.x;
				cur->a.y = block_tr.y;
				cur->b.x = block_bl.x;
				cur->b.y = block_bl.y;
				cur->n.x = -1;
				cur->n.y = 0;
				cur->next = ptr->segment_list;
				if (ptr->segment_list) {
					ptr->segment_list->prev = cur;
				}
				ptr->segment_list = cur;
			}

			if (!(flags & (TDS_BLOCK_TYPE_LTSLOPE | TDS_BLOCK_TYPE_RTSLOPE))) {
				/* Out-facing up segment. */
				cur = tds_malloc(sizeof *cur);
				cur->a.x = block_tr.x;
				cur->a.y = block_tr.y;
				cur->b.x = block_bl.x;
				cur->b.y = block_tr.y;
				cur->n.x = 0;
				cur->n.y = 1;
				cur->next = ptr->segment_list;
				if (ptr->segment_list) {
					ptr->segment_list->prev = cur;
				}
				ptr->segment_list = cur;
			}

			if (!(flags & (TDS_BLOCK_TYPE_LBSLOPE | TDS_BLOCK_TYPE_RBSLOPE))) {
				/* Out-facing down segment. */
				cur = tds_malloc(sizeof *cur);
				cur->a.x = block_bl.x;
				cur->a.y = block_bl.y;
				cur->b.x = block_tr.x;
				cur->b.y = block_bl.y;
				cur->n.x = 0;
				cur->n.y = -1;
				cur->next = ptr->segment_list;
				if (ptr->segment_list) {
					ptr->segment_list->prev = cur;
				}
				ptr->segment_list = cur;
			}

			if (flags & TDS_BLOCK_TYPE_LTSLOPE) {
				cur = tds_malloc(sizeof *cur);
				cur->a.x = block_bl.x;
				cur->a.y = block_bl.y;
				cur->b.x = block_tr.x;
				cur->b.y = block_tr.y;
				cur->n.x = -1;
				cur->n.y = 1;
				cur->next = ptr->segment_list;
				if (ptr->segment_list) {
					ptr->segment_list->prev = cur;
				}
				ptr->segment_list = cur;
			}

			if (flags & TDS_BLOCK_TYPE_RTSLOPE) {
				cur = tds_malloc(sizeof *cur);
				cur->a.x = block_tr.x;
				cur->a.y = block_bl.y;
				cur->b.x = block_bl.x;
				cur->b.y = block_tr.y;
				cur->n.x = 1;
				cur->n.y = 1;
				cur->next = ptr->segment_list;
				if (ptr->segment_list) {
					ptr->segment_list->prev = cur;
				}
				ptr->segment_list = cur;
			}

			if (flags & TDS_BLOCK_TYPE_RBSLOPE) {
				cur = tds_malloc(sizeof *cur);
				cur->a.x = block_bl.x;
				cur->a.y = block_bl.y;
				cur->b.x = block_tr.x;
				cur->b.y = block_tr.y;
				cur->n.x = 1;
				cur->n.y = -1;
				cur->next = ptr->segment_list;
				if (ptr->segment_list) {
					ptr->segment_list->prev = cur;
				}
				ptr->segment_list = cur;
			}

			if (flags & TDS_BLOCK_TYPE_LBSLOPE) {
				cur = tds_malloc(sizeof *cur);
				cur->a.x = block_bl.x;
				cur->a.y = block_tr.y;
				cur->b.x = block_tr.x;
				cur->b.y = block_bl.y;
				cur->n.x = -1;
				cur->n.y = -1;
				cur->next = ptr->segment_list;
				if (ptr->segment_list) {
					ptr->segment_list->prev = cur;
				}
				ptr->segment_list = cur;
			}
		}

		cur_block = cur_block->next;
	}

	tds_logf(TDS_LOG_DEBUG, "Starting linear reduction phase.\n");

	/* We will use a brute-force approach to keep the code small as this is not going to be called very often. */
	int complete = 1, iterations = 0;

	while (complete) {
		complete = 0;

		struct tds_world_segment* reduction_cur = NULL, *reduction_target = ptr->segment_list, *tmp = NULL;

		/* In this pass, we consider all segments for reduction with other segments. If any actual reduction is done, we set the flag for another pass. */
		tds_logf(TDS_LOG_DEBUG, "Starting linear reduction subphase iteration %d\n", iterations);

		while (reduction_target) {
			reduction_cur = ptr->segment_list;

			while (reduction_cur) {
				/* Three nested while loops for linear reduction and n^2 traversal of a linked list. Crazy. */

				if (reduction_cur == reduction_target) {
					reduction_cur = reduction_cur->next;
					continue;
				}

				if (!tds_vec2_cmpi(reduction_cur->n, 0, 1) && !tds_vec2_cmpi(reduction_target->n, 0, 1)) {
					/* Both of these lines are horizontal, and each segment's x2 is less than the x1. */

					if (reduction_cur->a.y != reduction_target->a.y) {
						reduction_cur = reduction_cur->next;
						continue;
					}

					if (reduction_cur->b.x == reduction_target->a.x + 1) {
						/* reduction_cur is on the right and we can reduce. */
						reduction_target->a.x = reduction_cur->a.x;

						if (reduction_cur->prev) {
							reduction_cur->prev->next = reduction_cur->next;
						} else {
							ptr->segment_list = reduction_cur->next;
						}

						if (reduction_cur->next) {
							reduction_cur->next->prev = reduction_cur->prev;
						}

						tmp = reduction_cur->next;
						tds_free(reduction_cur);
						reduction_cur = tmp;
						complete = 1;
						continue;
					}

					if (reduction_target->b.x == reduction_cur->a.x + 1) {
						/* reduction_target is on the right and we can reduce. */
						reduction_target->b.x = reduction_cur->b.x;

						if (reduction_cur->prev) {
							reduction_cur->prev->next = reduction_cur->next;
						} else {
							ptr->segment_list = reduction_cur->next;
						}

						if (reduction_cur->next) {
							reduction_cur->next->prev = reduction_cur->prev;
						}

						tmp = reduction_cur->next;
						tds_free(reduction_cur);
						reduction_cur = tmp;
						complete = 1;
						continue;
					}
				}

				if (!tds_vec2_cmpi(reduction_cur->n, 0, -1) && !tds_vec2_cmpi(reduction_target->n, 0, -1)) {
					/* Both of these lines are horizontal, and each segment's x2 is greater than the x1. */

					if (reduction_cur->a.y != reduction_target->a.y) {
						reduction_cur = reduction_cur->next;
						continue;
					}

					if (reduction_cur->a.x == reduction_target->b.x + 1) {
						/* reduction_cur is on the right and we can reduce. */
						reduction_target->b.x = reduction_cur->b.x;

						if (reduction_cur->prev) {
							reduction_cur->prev->next = reduction_cur->next;
						} else {
							ptr->segment_list = reduction_cur->next;
						}

						if (reduction_cur->next) {
							reduction_cur->next->prev = reduction_cur->prev;
						}

						tmp = reduction_cur->next;
						tds_free(reduction_cur);
						reduction_cur = tmp;
						complete = 1;
						continue;
					}

					if (reduction_target->a.x == reduction_cur->b.x + 1) {
						/* reduction_target is on the right and we can reduce. */
						reduction_target->a.x = reduction_cur->a.x;

						if (reduction_cur->prev) {
							reduction_cur->prev->next = reduction_cur->next;
						} else {
							ptr->segment_list = reduction_cur->next;
						}

						if (reduction_cur->next) {
							reduction_cur->next->prev = reduction_cur->prev;
						}

						tmp = reduction_cur->next;
						tds_free(reduction_cur);
						reduction_cur = tmp;
						complete = 1;
						continue;
					}
				}

				if (!tds_vec2_cmpi(reduction_cur->n, 1, 0) && !tds_vec2_cmpi(reduction_target->n, 1, 0)) {
					/* Both of these lines are vertical, and each segment's y2 is greater than the y1. */

					if (reduction_cur->a.x != reduction_target->a.x) {
						reduction_cur = reduction_cur->next;
						continue;
					}

					if (reduction_cur->b.y == reduction_target->a.y + 1) {
						/* reduction_cur is on the bottom and we can reduce. */
						reduction_target->a.y = reduction_cur->a.y;

						if (reduction_cur->prev) {
							reduction_cur->prev->next = reduction_cur->next;
						} else {
							ptr->segment_list = reduction_cur->next;
						}

						if (reduction_cur->next) {
							reduction_cur->next->prev = reduction_cur->prev;
						}

						tmp = reduction_cur->next;
						tds_free(reduction_cur);
						reduction_cur = tmp;
						complete = 1;
						continue;
					}

					if (reduction_target->b.y == reduction_cur->a.y + 1) {
						/* reduction_target is on the bottom and we can reduce. */
						reduction_target->b.y = reduction_cur->b.y;

						if (reduction_cur->prev) {
							reduction_cur->prev->next = reduction_cur->next;
						} else {
							ptr->segment_list = reduction_cur->next;
						}

						if (reduction_cur->next) {
							reduction_cur->next->prev = reduction_cur->prev;
						}

						tds_free(reduction_cur);
						tmp = reduction_cur->next;
						tds_free(reduction_cur);
						reduction_cur = tmp;
						complete = 1;
						continue;
					}
				}

				if (!tds_vec2_cmpi(reduction_cur->n, -1, 0) && !tds_vec2_cmpi(reduction_target->n, -1, 0)) {
					/* Both of these lines are vertical, and each segment's y2 is less than the y1. */

					if (reduction_cur->a.x != reduction_target->a.x) {
						reduction_cur = reduction_cur->next;
						continue;
					}

					if (reduction_cur->b.y == reduction_target->a.y + 1) {
						/* reduction_cur is on the top and we can reduce. */
						reduction_target->a.y = reduction_cur->a.y;

						if (reduction_cur->prev) {
							reduction_cur->prev->next = reduction_cur->next;
						} else {
							ptr->segment_list = reduction_cur->next;
						}

						if (reduction_cur->next) {
							reduction_cur->next->prev = reduction_cur->prev;
						}

						tmp = reduction_cur->next;
						tds_free(reduction_cur);
						reduction_cur = tmp;
						complete = 1;
						continue;
					}

					if (reduction_target->b.y == reduction_cur->a.y + 1) {
						/* reduction_target is on the top and we can reduce. */
						reduction_target->b.y = reduction_cur->b.y;

						if (reduction_cur->prev) {
							reduction_cur->prev->next = reduction_cur->next;
						} else {
							ptr->segment_list = reduction_cur->next;
						}

						if (reduction_cur->next) {
							reduction_cur->next->prev = reduction_cur->prev;
						}

						tmp = reduction_cur->next;
						tds_free(reduction_cur);
						reduction_cur = tmp;
						complete = 1;
						continue;
					}
				}

				reduction_cur = reduction_cur->next;
			}

			reduction_target = reduction_target->next;
		}

		++iterations;
	}

	tds_logf(TDS_LOG_DEBUG, "Finished linear reduction phase in %d passes.\n", iterations);

	cur = ptr->segment_list;

	/* We iterate through the segment list twice. Once to grab the size of the list, and another to copy the segment data to the temporary buffer. */

	int segment_count = 0;

	while (cur) {
		segment_count++;
		cur = cur->next;
	}

	struct tds_vertex* segment_verts = tds_malloc(segment_count * sizeof(struct tds_vertex) * 2);
	cur = ptr->segment_list;

	int i = 0;

	while (cur) {
		/* the vertex buffer should be appropriately sized by floats, we will deal with translating it later */
		/* TODO : segment vbos still rely on floating-point niceness, consider moving VBOs to be relative and translating them later in the render process */

		struct tds_vertex verts[] = {
			{(float) cur->a.x / 16.0f, (float) cur->a.y / 16.0f, 0.0f, cur->n.x, cur->n.y}, /* We hide the normal in the texcoords, saving some time. */
			{(float) cur->b.x / 16.0f, (float) cur->b.y, 0.0f, cur->n.x, cur->n.y},
		};

		if (i >= segment_count) {
			break;
		}

		segment_verts[2 * i] = verts[0];
		segment_verts[2 * i + 1] = verts[1];

		cur = cur->next;

		++i;
	}

	ptr->segment_vb = tds_vertex_buffer_create(segment_verts, segment_count * 2, GL_LINES);
	tds_free(segment_verts);
}

void _tds_world_generate_quadtree(struct tds_world* ptr) {
	if (ptr->quadtree) {
		tds_quadtree_free(ptr->quadtree);
	}

	ptr->quadtree = tds_quadtree_create(0, 0xFFFFFFFF, 0xFFFFFFFF, 0); /* uses a lot of space, log(n) though so probably no big deal */

	struct tds_world_hblock* cur = ptr->block_list_head;

	while (cur) {
		tds_quadtree_insert(ptr->quadtree, cur->pos, cur->dim, cur);
		cur = cur->next;
	}
}

void tds_world_load(struct tds_world* ptr, uint8_t* blocks, int w, int h, int x, int y) {
	/* destroy anything left over -- although there are offsets any previous hblocks in this range could be in danger. */
	tds_world_init(ptr);

	ptr->block_list_head = NULL;
	ptr->block_list_tail = NULL;

	struct tds_world_hblock* cur = NULL;

	/* we're not going to hold on to block data or copy it. generate hblocks here and call it good. */
	for (int cy = y; cy < y + h; ++cy) {
		int cur_id = -1, cur_blockx = 0;

		for (int cx = x; cx < x + w; ++cx) {
			if (blocks[cx + cy * w] != cur_id) {
				if (cur_id > 0) {
					/* emit block starting at cur_blockx, ending at cx - 1 */

					cur = tds_malloc(sizeof *cur);
					cur->pos.x = cur_blockx * 16;
					cur->pos.y = cy * 16;
					cur->dim.x = (cx - cur_blockx) * 16;
					cur->dim.y = 16;
					cur->id = cur_id;

					cur->next = NULL;

					if (ptr->block_list_tail) {
						ptr->block_list_tail->next = cur;
						ptr->block_list_tail = cur;
					} else {
						ptr->block_list_head = ptr->block_list_tail = cur;
					}
				}

				cur_id = blocks[cx + cy * w];
				cur_blockx = cx;
			}
		}

		if (cur_id > 0) {
			/* emit block starting at cur_blockx, ending at x + w */

			cur = tds_malloc(sizeof *cur);
			cur->pos.x = cur_blockx * 16;
			cur->pos.y = cy * 16;
			cur->dim.x = ((x + w + 1) - cur_blockx) * 16;
			cur->dim.y = 16;
			cur->id = cur_id;

			cur->next = NULL;

			if (ptr->block_list_tail) {
				ptr->block_list_tail->next = cur;
				ptr->block_list_tail = cur;
			} else {
				ptr->block_list_head = ptr->block_list_tail = cur;
			}
		}
	}

	/* insert hblocks into quadtree */
	_tds_world_generate_quadtree(ptr);

	/* construct segments */
	_tds_world_generate_segments(ptr);

	/* generate VBOs for active hblocks */
	cur = ptr->block_list_head;

	while (cur) {
		tds_logf(TDS_LOG_DEBUG, "Located hblock in world data with pos=%d,%d dim=%d,%d id=%d\n", cur->pos.x, cur->pos.y, cur->dim.x, cur->dim.y, cur->id);

		float right = cur->dim.x;

		struct tds_vertex vert_list[] = { /* need to be translated, VBOs centered on lower-left hblock origin */
			{ 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
			{ right / 16.0f, 0.0f, 0.0f, right / 16.0f, 0.0f },
			{ right / 16.0f, 1.0f, 0.0f, right / 16.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
			{ right / 16.0f, 0.0f, 0.0f, right / 16.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		};

		cur->vb = tds_vertex_buffer_create(vert_list, 6, GL_TRIANGLES);
		cur = cur->next;
	}
}
