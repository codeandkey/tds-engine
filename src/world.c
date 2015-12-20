#include "world.h"
#include "log.h"
#include "memory.h"
#include "engine.h"

#include <GLXW/glxw.h>

static void _tds_world_generate_hblocks(struct tds_world* ptr);
static void _tds_world_generate_segments(struct tds_world* ptr);

struct tds_world* tds_world_create(void) {
	struct tds_world* output = tds_malloc(sizeof *output);

	output->width = output->height = 0;
	output->block_list_head = output->block_list_tail = 0;
	output->buffer = 0;

	return output;
}

void tds_world_free(struct tds_world* ptr) {
	if (ptr->buffer) {
		for (int i = 0; i < ptr->height; ++i) {
			tds_free(ptr->buffer[i]);
		}

		tds_free(ptr->buffer);
	}

	if (ptr->block_list_head) {
		struct tds_world_hblock* cur = ptr->block_list_head, *tmp = 0;

		while (cur) {
			tmp = cur->next;
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

	tds_free(ptr);
}

void tds_world_init(struct tds_world* ptr, int width, int height) {
	if (ptr->buffer) {
		for (int i = 0; i < height; ++i) {
			tds_free(ptr->buffer[i]);
		}

		tds_free(ptr->buffer);
	}

	ptr->buffer = tds_malloc(sizeof ptr->buffer[0] * height);

	ptr->width = width;
	ptr->height = height;

	for (int y = 0; y < height; ++y) {
		ptr->buffer[y] = tds_malloc(sizeof ptr->buffer[0][0] * width);

		for (int x = 0; x < width; ++x) {
			ptr->buffer[y][x] = 0;
		}
	}
}

void tds_world_load(struct tds_world* ptr, const uint8_t* block_buffer, int width, int height) {
	tds_world_init(ptr, width, height);

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			ptr->buffer[y][x] = block_buffer[y * width + x];
		}
	}

	_tds_world_generate_hblocks(ptr);
}

void tds_world_save(struct tds_world* ptr, uint8_t* block_buffer, int width, int height) {
	/* Simply copying the buffer. The buffer will _always_ be up to date. */

	if (width != ptr->width || height != ptr->height) {
		tds_logf(TDS_LOG_CRITICAL, "World size mismatch.\n");
		return;
	}

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			block_buffer[y * width + x] = ptr->buffer[y][x];
		}
	}
}

void tds_world_set_block(struct tds_world* ptr, int x, int y, uint8_t block) {
	if (x >= ptr->width || x < 0 || y >= ptr->height || y < 0) {
		tds_logf(TDS_LOG_WARNING, "World index out of bounds.\n");
		return;
	}

	ptr->buffer[y][x] = block;
	_tds_world_generate_hblocks(ptr); /* This is not the most efficient way to do this, but it really shouldn't matter with small worlds. */

	/* If worlds end up not being small for some reason, we can regenerate only the block which the target resided in along with it's neighbors. */
	/* This shouldn't be called often anyway, so.. not a big deal. */
}

uint8_t tds_world_get_block(struct tds_world* ptr, int x, int y) {
	return ptr->buffer[y][x];
}

static void _tds_world_generate_hblocks(struct tds_world* ptr) {
	/* Perhaps one of the more important functions : regenerate the horizontally reduced blocks */

	if (ptr->block_list_head) {
		/* There are already blocks here. Into the trash! */
		struct tds_world_hblock* cur = ptr->block_list_head, *tmp = 0;

		while (cur) {
			tds_vertex_buffer_free(cur->vb);
			tmp = cur->next;
			tds_free(cur);
			cur = tmp;
		}

		ptr->block_list_head = ptr->block_list_tail = NULL;
	}

	for (int y = 0; y < ptr->height; ++y) {
		uint8_t cur_type = 0;
		int block_length = 0, block_x = -1;
		struct tds_world_hblock* tmp_block = NULL;

		for (int x = 0; x < ptr->width; ++x) {
			if (ptr->buffer[y][x] != cur_type) {
				if (block_length > 0 && cur_type) {
					/* Extract a block. */
					tmp_block = tds_malloc(sizeof *tmp_block);

					tmp_block->next = 0;
					tmp_block->x = block_x;
					tmp_block->y = y;
					tmp_block->w = block_length;
					tmp_block->id = cur_type;

					if (ptr->block_list_tail) {
						ptr->block_list_tail->next = tmp_block;
					} else {
						ptr->block_list_head = tmp_block;
					}

					ptr->block_list_tail = tmp_block;
				}

				cur_type = ptr->buffer[y][x];
				block_x = x;
				block_length = 1;
			} else {
				block_length++;
			}
		}

		if (!cur_type) {
			continue; /* The last block is air, no reason to generate it */
		}

		/* We also extract a block afterwards, or we'll miss some */
		/* Extract a block. */
		tmp_block = tds_malloc(sizeof *tmp_block);

		tmp_block->next = 0;
		tmp_block->x = block_x;
		tmp_block->y = y;
		tmp_block->w = block_length;
		tmp_block->id = cur_type;

		if (ptr->block_list_tail) {
			ptr->block_list_tail->next = tmp_block;
		} else {
			ptr->block_list_head = tmp_block;
		}

		ptr->block_list_tail = tmp_block;
	}

	/* At this time we will also generate VBOs for each hblock, so that tds_render doesn't have to. */

	struct tds_world_hblock* hb_cur = ptr->block_list_head;
	while (hb_cur) {
		struct tds_vertex vert_list[] = {
			{ -hb_cur->w * TDS_WORLD_BLOCK_SIZE / 2.0f, TDS_WORLD_BLOCK_SIZE / 2.0f, 0.0f, 0.0f, 1.0f },
			{ hb_cur->w * TDS_WORLD_BLOCK_SIZE / 2.0f, -TDS_WORLD_BLOCK_SIZE / 2.0f, 0.0f, hb_cur->w, 0.0f },
			{ hb_cur->w * TDS_WORLD_BLOCK_SIZE / 2.0f, TDS_WORLD_BLOCK_SIZE / 2.0f, 0.0f, hb_cur->w, 1.0f },
			{ -hb_cur->w * TDS_WORLD_BLOCK_SIZE / 2.0f, TDS_WORLD_BLOCK_SIZE / 2.0f, 0.0f, 0.0f, 1.0f },
			{ hb_cur->w * TDS_WORLD_BLOCK_SIZE / 2.0f, -TDS_WORLD_BLOCK_SIZE / 2.0f, 0.0f, hb_cur->w, 0.0f },
			{ -hb_cur->w * TDS_WORLD_BLOCK_SIZE / 2.0f, -TDS_WORLD_BLOCK_SIZE / 2.0f, 0.0f, 0.0f, 0.0f },
		};

		hb_cur->vb = tds_vertex_buffer_create(vert_list, 6, GL_TRIANGLES);
		hb_cur = hb_cur->next;
	}
}

int tds_world_get_overlap_fast(struct tds_world* ptr, struct tds_object* obj, float* x, float* y, float* w, float* h) {
	/* Another important function. Intersection testing with axis-aligned objects. */

	float obj_left = obj->x - obj->cbox_width / 2.0f;
	float obj_right = obj->x + obj->cbox_width / 2.0f;
	float obj_bottom = obj->y - obj->cbox_height / 2.0f;
	float obj_top = obj->y + obj->cbox_height / 2.0f;

	if (obj->angle) {
		tds_logf(TDS_LOG_WARNING, "The target object is not axis-aligned. Using a wider bounding box than normal to accommadate.\n");

		float diagonal = sqrtf(pow(obj->cbox_width, 2) + pow(obj->cbox_height, 2)) / 2.0f;

		obj_left = obj->x - diagonal;
		obj_right = obj->x + diagonal;
		obj_bottom = obj->y - diagonal;
		obj_top = obj->y + diagonal;
	}

	/* The world block coordinates will be treated as centers. The block at [0, 0] will be centered on the origin. */
	struct tds_world_hblock* cblock = ptr->block_list_head;

	while (cblock) {
		if (!tds_block_map_get(tds_engine_global->block_map_handle, cblock->id).solid) {
			continue;
		}

		float cblock_left = (cblock->x - 0.5f - ptr->width / 2.0f) * TDS_WORLD_BLOCK_SIZE;
		float cblock_right = (cblock->x + cblock->w - 0.5f - ptr->width / 2.0f) * TDS_WORLD_BLOCK_SIZE;
		float cblock_top = (cblock->y + 0.5f - ptr->height / 2.0f) * TDS_WORLD_BLOCK_SIZE;
		float cblock_bottom = (cblock->y - ptr->height / 2.0f - 0.5f) * TDS_WORLD_BLOCK_SIZE;

		if (obj_left > cblock_right) {
			cblock = cblock->next;
			continue;
		}

		if (obj_right < cblock_left) {
			cblock = cblock->next;
			continue;
		}

		if (obj_top < cblock_bottom) {
			cblock = cblock->next;
			continue;
		}

		if (obj_bottom > cblock_top) {
			cblock = cblock->next;
			continue;
		}

		if (x) {
			*x = (cblock->x - 0.5f - ptr->width / 2.0f) * TDS_WORLD_BLOCK_SIZE;
		}

		if (y) {
			*y = (cblock->y + 0.5f - ptr->height / 2.0f) * TDS_WORLD_BLOCK_SIZE;
		}

		if (w) {
			*w = cblock->w * TDS_WORLD_BLOCK_SIZE;
		}

		if (h) {
			*h = TDS_WORLD_BLOCK_SIZE;
		}

		return 1;
	}

	return 0;
}

void _tds_world_generate_segments(struct tds_world* ptr) {
	struct tds_world_segment* head = ptr->segment_list, *cur = NULL;

	while (head) {
		cur = head->next;
		tds_free(head);
		head = cur;
	}

	/* TODO : this whole thing */
}
