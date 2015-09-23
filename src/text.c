#include "text.h"
#include "memory.h"
#include "sprite.h"
#include "log.h"

#include <stdlib.h>

struct tds_text* tds_text_create(void) {
	struct tds_text* output = tds_malloc(sizeof(struct tds_text));

	output->head = output->tail = NULL;
	output->size = 0;

	return output;
}

void tds_text_free(struct tds_text* ptr) {
	tds_text_clear(ptr);
	tds_free(ptr);
}

void tds_text_submit(struct tds_text* ptr, struct tds_text_batch* batch) {
	struct tds_text_batch_entry* next_element = tds_malloc(sizeof(struct tds_text_batch_entry));

	next_element->data = *batch;
	next_element->prev = ptr->tail;
	next_element->next = NULL;

	if (ptr->tail) {
		ptr->tail->next = next_element;
	}

	ptr->tail = next_element;

	if (!ptr->head) {
		ptr->head = next_element;
	}

	ptr->size++;
}

void tds_text_clear(struct tds_text* ptr) {
	struct tds_text_batch_entry* entry = ptr->head, *tmp = NULL;

	while (entry) {
		tmp = entry;
		entry = entry->next;

		tds_free(tmp);
	}

	ptr->head = NULL;
	ptr->tail = NULL;
	ptr->size = 0;
}

vec4* tds_text_batch_get_transform(struct tds_text_batch* ptr, int index) {
	/* We want to translate the glpyh to it's correct location on the line (post line-rotation) and then rotate the glyph image itself */

	float total_width = ptr->str_len * ptr->font->width;
	float prg = (float) index / (float) ptr->str_len;

	if (ptr->flags & TDS_TEXT_BATCH_FLAG_CENTERED) {
		prg -= 0.5f; /* Convert progress from [0,1] to [-0.5,0.5] */
	}

	float gx = prg * total_width * cosf(ptr->angle) + ptr->x, gy = prg * total_width * sinf(ptr->angle) + ptr->y;

	mat4x4 id, pos, rot;
	mat4x4_identity(ptr->transform);
	mat4x4_identity(id);

	mat4x4_translate(pos, gx, gy, ptr->z);
	mat4x4_rotate_Z(rot, id, ptr->angle);

	mat4x4_mul(ptr->transform, pos, rot);

	return ptr->transform;
}
