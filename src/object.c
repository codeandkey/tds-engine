#include "object.h"
#include "log.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>

struct tds_object* tds_object_create(struct tds_object_type* type, struct tds_handle_manager* hmgr, struct tds_sprite* sprite, float x, float y, float z, void* data) {
	struct tds_object* output = tds_malloc(sizeof(struct tds_object));

	output->type_name = type->type_name;

	output->func_init = type->func_init;
	output->func_update = type->func_update;
	output->func_draw = type->func_draw;
	output->func_msg = type->func_msg;
	output->func_destroy = type->func_destroy;

	output->x = x;
	output->y = y;
	output->z = z;
	output->angle = 0.0f;
	output->layer = 0; /* Layers are rendered with lower numbers on bottom, higher numbers on top. */

	output->r = output->g = output->b = output->a = 1.0f;

	output->visible = (sprite != 0);
	output->sprite_handle = sprite;

	output->object_data = type->data_size ? tds_malloc(type->data_size) : NULL;
	output->object_handle = tds_handle_manager_get_new(hmgr, output);
	output->hmgr = hmgr;

	if (data && output->object_data) {
		memcpy(output->object_data, data, type->data_size);
	}

	if (output->func_init) {
		(output->func_init)(output);
	}

	tds_logf(TDS_LOG_MESSAGE, "created object with handle %d, sprite %X\n", output->object_handle, (unsigned long) sprite);

	return output;
}

void tds_object_free(struct tds_object* ptr) {
	if (ptr->func_destroy) {
		(ptr->func_destroy)(ptr);
	}

	if (ptr->hmgr) {
		tds_handle_manager_set(ptr->hmgr, ptr->object_handle, NULL);
	}

	if (ptr->object_data) {
		tds_free(ptr->object_data);
	}

	tds_free(ptr);
}

void tds_object_set_sprite(struct tds_object* ptr, struct tds_sprite* sprite) {
	ptr->sprite_handle = sprite;
}

void tds_object_send_msg(struct tds_object* ptr, int handle, int msg, void* data) {
	struct tds_object* target = tds_handle_manager_get(ptr->hmgr, handle);

	target->func_msg(target, ptr, msg, data);
}

float* tds_object_get_transform(struct tds_object* ptr) {
	mat4x4 id, pos, rot;

	mat4x4_identity(ptr->transform);
	mat4x4_identity(id);

	mat4x4_translate(pos, ptr->x, ptr->y, ptr->z);
	mat4x4_rotate_Z(rot, id, ptr->angle);

	mat4x4_mul(ptr->transform, ptr->transform, rot);
	mat4x4_mul(ptr->transform, ptr->transform, pos);

	return (float*) ptr->transform;
};
