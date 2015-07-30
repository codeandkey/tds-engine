#pragma once
#include "vertex.h"

/* Since most shapes will not have many shared verts, we will not use index buffers. */

struct tds_vertex_buffer {
	unsigned int vbo, vao;
	unsigned int vertex_count;
	unsigned int render_mode;
};

struct tds_vertex_buffer* tds_vertex_buffer_create(struct tds_vertex* verts, int count, unsigned int render_mode);
void tds_vertex_buffer_free(struct tds_vertex_buffer* ptr);

void tds_vertex_buffer_bind(struct tds_vertex_buffer* ptr);
