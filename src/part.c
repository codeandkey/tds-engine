#include "part.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>

#include <GLXW/glxw.h>

struct tds_part* tds_part_create(void) {
	struct tds_part* output = tds_malloc(sizeof *output);
	output->emitters = NULL;

	return output;
}

void tds_part_free(struct tds_part* ptr) {
	tds_part_flush(ptr);
	tds_free(ptr);
}

struct tds_part_emitter* tds_part_add(struct tds_part* ptr, struct tds_part_type* type, int part_max) {
	struct tds_part_emitter* output = tds_malloc(sizeof *output);
	void* bufdata = NULL;

	output->type = type;
	output->verts = NULL;
	output->vert_count = part_max;

	glGenBuffers(1, &output->vbo);

	/* TODO */

	glBindBuffer(GL_ARRAY_BUFFER, output->vbo);
	bufdata = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	memset(bufdata, 0, sizeof(struct tds_vertex) * part_max);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	output->next = ptr->emitters;
	output->prev = NULL;

	if (output->next) {
		output->next->prev = output;
	}

	ptr->emitters = output;

	return output;
}

void tds_part_remove(struct tds_part* ptr, struct tds_part_emitter* emitter) {
	if (emitter->prev) {
		emitter->prev->next = emitter->next;
	}

	if (emitter->next) {
		emitter->next->prev = emitter->prev;
	}

	if (emitter == ptr->emitters) {
		ptr->emitters = emitter->next;
	}

	glDeleteBuffers(1, &emitter->vbo);
	glDeleteVertexArrays(1, &emitter->vao);

	tds_free(emitter);
}

void tds_part_flush(struct tds_part* ptr) {
	struct tds_part_emitter* cur = ptr->emitters, *tmp = NULL;

	while (cur) {
		tmp = cur->next;
		tds_free(cur);
		glDeleteBuffers(1, &cur->vbo);
		glDeleteVertexArrays(1, &cur->vao);
		cur = tmp;
	}

	ptr->emitters = NULL;
}
