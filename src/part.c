#include "part.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>

#include <GLXW/glxw.h>

#define rand_p(p) (((float) (rand() % p) / (float) p) * 2.0f - 1.0f)

struct tds_part_system* tds_part_system_create(struct tds_part_type type) {
	struct tds_part_system* output = tds_malloc(sizeof *output);

	output->type = type;
	output->write_offset = 0;

	glGenBuffers(1, &output->vbo);
	glGenVertexArrays(1, &output->vao);

	glBindBuffer(GL_ARRAY_BUFFER, output->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(struct tds_part_vertex) * type.max_particles, NULL, GL_STATIC_DRAW); /* TODO : consider changing GL_STATIC_DRAW */

	glBindVertexArray(output->vao);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct tds_part_vertex), (void*) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(struct tds_part_vertex), (void*) (sizeof(float) * 2));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct tds_part_vertex), (void*) (sizeof(float) * 6));

	return output;
}

void tds_part_system_free(struct tds_part_system* ptr) {
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &ptr->vao);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ptr->vbo);

	tds_free(ptr);
}

void tds_part_system_flush(struct tds_part_system* ptr) {
	glBindBuffer(GL_ARRAY_BUFFER, ptr->vbo);
	struct tds_part_vertex* bufmap = glMapBuffer(ptr->vbo, GL_READ_WRITE);

	memset(bufmap, 0, sizeof *bufmap * ptr->type.max_particles);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void tds_part_system_emit(struct tds_part_system* ptr, int num, float x, float y, float* color, float* speed) {
	glBindBuffer(GL_ARRAY_BUFFER, ptr->vbo);
	struct tds_part_vertex* bufmap = glMapBuffer(ptr->vbo, GL_READ_WRITE);
	int write_index = ptr->write_offset;

	for (int i = 0; i < num; ++i) {
		bufmap[write_index].x = x + ptr->type.position_offset[0] + rand_p(ptr->type.rand_precision) * ptr->type.position_variance[0];
		bufmap[write_index].y = y + ptr->type.position_offset[1] + rand_p(ptr->type.rand_precision) * ptr->type.position_variance[1];
		bufmap[write_index].r = color[0] + ptr->type.color_offset[0] + rand_p(ptr->type.rand_precision) * ptr->type.color_variance[0];
		bufmap[write_index].g = color[1] + ptr->type.color_offset[1] + rand_p(ptr->type.rand_precision) * ptr->type.color_variance[1];
		bufmap[write_index].b = color[2] + ptr->type.color_offset[2] + rand_p(ptr->type.rand_precision) * ptr->type.color_variance[2];
		bufmap[write_index].a = color[3] + ptr->type.color_offset[3] + rand_p(ptr->type.rand_precision) * ptr->type.color_variance[3];
		bufmap[write_index].dx = speed[0] + ptr->type.speed_offset[0] + rand_p(ptr->type.rand_precision) * ptr->type.speed_variance[0];
		bufmap[write_index].dy = speed[1] + ptr->type.speed_offset[1] + rand_p(ptr->type.rand_precision) * ptr->type.speed_variance[1];

		if (++write_index >= ptr->type.max_particles) {
			write_index = 0;
		}
	}

	ptr->write_offset = write_index;
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

struct tds_part_manager* tds_part_manager_create(void) {
	struct tds_part_manager* output = tds_malloc(sizeof *output);

	const char* varyings[] = {
		"part_out.pos"
		"part_out.color"
		"part_out.speed"
	};

	output->systems = NULL;
	output->shader_part_render = tds_shader_create(TDS_PART_MANAGER_SHADER_RENDER_VS, TDS_PART_MANAGER_SHADER_RENDER_GS, TDS_PART_MANAGER_SHADER_RENDER_FS);
	//output->shader_part_animate = tds_shader_create_tfb(TDS_PART_MANAGER_SHADER_ANIMATE_VS, TDS_PART_MANAGER_SHADER_ANIMATE_GS, TDS_PART_MANAGER_SHADER_ANIMATE_FS, varyings, sizeof varyings / sizeof *varyings);

	return output;
}

void tds_part_manager_free(struct tds_part_manager* ptr) {
	tds_part_manager_flush(ptr);

	tds_shader_free(ptr->shader_part_render);
	//tds_shader_free(ptr->shader_part_animate);

	tds_free(ptr);
}

void tds_part_manager_flush(struct tds_part_manager* ptr) {
	struct tds_part_system* tmp = NULL;

	while (ptr->systems) {
		tmp = ptr->systems->next;
		tds_part_system_free(ptr->systems);
		ptr->systems = tmp;
	}
}

void tds_part_manager_add(struct tds_part_manager* ptr, struct tds_part_system* sys) {
	sys->next = ptr->systems;
	ptr->systems = sys;
}

void tds_part_manager_animate(struct tds_part_manager* ptr) {
}

void tds_part_manager_render(struct tds_part_manager* ptr) {
	tds_shader_bind(ptr->shader_part_render);
	struct tds_part_system* cur = ptr->systems;

	while (cur) {
		glBindVertexArray(cur->vao);

		switch (cur->type.blending) {
		case TDS_PART_TYPE_BLENDING_NORMAL:
			glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
			break;
		case TDS_PART_TYPE_BLENDING_ADDITIVE:
			glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
			break;
		}

		glDrawArrays(GL_POINTS, 0, cur->type.max_particles);
		glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	}
}
