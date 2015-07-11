#pragma once

#include "sprite.h"
#include "handle.h"
#include "linmath.h"

struct tds_object {
	struct tds_sprite* sprite_handle;
	const char* type_name;

	int visible, layer;
	float x, y, z, angle, r, g, b, a;

	mat4x4 transform;

	void (*func_init)(struct tds_object* ptr);
	void (*func_destroy)(struct tds_object* ptr);
	void (*func_update)(struct tds_object* this);
	void (*func_draw)(struct tds_object* this);
	void (*func_msg)(struct tds_object* ptr, struct tds_object* from, int msg, void* param);

	void* object_data; /* set to a custom structure per object type. */
	int object_handle;

	struct tds_handle_manager* hmgr;
};

struct tds_object_type {
	const char* type_name;
	int data_size;

	void (*func_init)(struct tds_object* ptr);
	void (*func_destroy)(struct tds_object* ptr);
	void (*func_update)(struct tds_object* ptr);
	void (*func_draw)(struct tds_object* ptr);
	void (*func_msg)(struct tds_object* ptr, struct tds_object* from, int msg, void* param);
};

struct tds_object* tds_object_create(struct tds_object_type* type, struct tds_handle_manager* hmgr, struct tds_sprite* sprite, float x, float y, float z, void* initial_data);
void tds_object_free(struct tds_object* ptr);

void tds_object_set_sprite(struct tds_object* ptr, struct tds_sprite* sprite);
void tds_object_send_msg(struct tds_object* ptr, int handle, int msg, void* data);
float* tds_object_get_transform(struct tds_object* ptr);

/* instead of encapsulating object functions (via more functions), systems and the like should just directly call the function pointers. */
/* they should be fe to call, they are checked by the creation functions */
