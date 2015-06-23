#include "vertex_buffer.h"
#include "log.h"
#include "memory.h"

#include <GLXW/glxw.h>

struct tds_vertex_buffer* tds_vertex_buffer_create(struct tds_vertex* verts, int count, unsigned int render_mode) {
	struct tds_vertex_buffer* output = tds_malloc(sizeof(struct tds_vertex_buffer));

	glGenBuffers(1, &output->vbo);
	glGenVertexArrays(1, &output->vao);

	glBindBuffer(GL_ARRAY_BUFFER, output->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(struct tds_vertex) * count, verts, GL_STATIC_DRAW);

	glBindVertexArray(output->vao);

	glEnableVertexAttribArray(0); /* position */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct tds_vertex), (void*) 0);

	glEnableVertexAttribArray(1); /* texture coordinates */
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct tds_vertex), (void*) (sizeof(float) * 3));

	glEnableVertexAttribArray(2); /* color */
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct tds_vertex), (void*) (sizeof(float) * 5));

	output->vertex_count = count;
	output->render_mode = render_mode;

	return output;
}

void tds_vertex_buffer_free(struct tds_vertex_buffer* ptr) {
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &ptr->vao);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ptr->vbo);

	tds_free(ptr);
}

void tds_vertex_buffer_bind(struct tds_vertex_buffer* ptr) {
	glBindVertexArray(ptr->vao);
}
