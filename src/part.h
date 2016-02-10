#pragma once

/*
 * TDS particle rendering is done via vertex buffers of points, transformed via GS to billboarded quads.
 * Divided into systems and emitters.
 * Emitters have a buffer of points to render, and systems define attributes shared between emitters (update func, texture) 
 */

#include "texture.h"
#include "vertex.h"

typedef void (*tds_part_update_func)(struct tds_vertex* verts, int vert_count);

struct tds_part_type {
	struct tds_texture* part_texture;
	tds_part_update_func part_update;
};

struct tds_part_emitter {
	struct tds_part_type* type;
	struct tds_vertex* verts;
	int vert_count;
	struct tds_part_emitter* next, *prev;
	unsigned int vbo, vao;
};

struct tds_part {
	struct tds_part_emitter* emitters;
};

struct tds_part* tds_part_create(void);
void tds_part_free(struct tds_part* ptr);

struct tds_part_emitter* tds_part_add(struct tds_part* ptr, struct tds_part_type* type, int part_max);
void tds_part_remove(struct tds_part* ptr, struct tds_part_emitter* emitter);

void tds_part_flush(struct tds_part* ptr);
