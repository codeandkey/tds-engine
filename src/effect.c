#include "effect.h"
#include "log.h"
#include "memory.h"
#include "render.h"
#include "camera.h"
#include "engine.h"

#include <GLXW/glxw.h>
#include <stdlib.h>

struct tds_effect* tds_effect_create(void) {
	struct tds_effect* output = tds_malloc(sizeof *output);
	output->list = NULL;
	return output;
}

void tds_effect_free(struct tds_effect* ptr) {
	tds_effect_flush(ptr);
	tds_free(ptr);
}

void tds_effect_push(struct tds_effect* ptr, struct tds_effect_type* type) {
	struct tds_effect_instance* inst = tds_malloc(sizeof *inst);

	inst->type = type;
	inst->next = ptr->list;
	inst->state = type->func_init();

	ptr->list = inst;

	tds_logf(TDS_LOG_DEBUG, "Pushed effect [%s] to tracker.\n", inst->type->name);
}

void tds_effect_flush(struct tds_effect* ptr) {
	struct tds_effect_instance* cur = ptr->list, *tmp = NULL;

	while (cur) {
		tmp = cur->next;
		cur->type->func_free(&cur->state);
		tds_free(cur);
		cur = tmp;
	}

	ptr->list = NULL;
}

void tds_effect_update(struct tds_effect* ptr) {
	struct tds_effect_instance* cur = ptr->list;

	while (cur) {
		cur->type->func_update(&cur->state);
		cur = cur->next;
	}
}

void tds_effect_render(struct tds_effect* ptr, struct tds_shader* shader) {
	struct tds_effect_instance* cur = ptr->list;

	mat4x4 final, transform;
	

	while (cur) {
		cur->type->func_render(&cur->state);

		/* cur has updated it's buffers with positions of particles and we are ready to render all of them. */
		/* We won't be messing with RTs or anything (we'll leave that to the calling subsystem) */

		for (int i = 0; i < cur->state.part_count; ++i) {
			mat4x4_translate(transform, cur->state.part_buf[i].x, cur->state.part_buf[i].y, 0.0f);
			mat4x4_mul(final, tds_engine_global->camera_handle->mat_transform, transform);

			tds_shader_set_color(shader, cur->state.part_buf[i].r, cur->state.part_buf[i].g, cur->state.part_buf[i].b, cur->state.part_buf[i].a);
			tds_shader_set_transform(shader, (float*) *final);

			glBindVertexArray(cur->state.vb->vao);
			glBindTexture(GL_TEXTURE_2D, cur->state.tex->gl_id);

			glDrawArrays(cur->state.vb->render_mode, 0, cur->state.vb->vertex_count);
		}

		cur = cur->next;
	}
}
