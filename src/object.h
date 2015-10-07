#pragma once

#include "sprite.h"
#include "handle.h"
#include "linmath.h"
#include "clock.h"
#include "sprite_cache.h"
#include "sound_source.h"

struct tds_object_param;

#define TDS_PARAM_KEYSIZE 32
#define TDS_PARAM_VALSIZE 32

#define TDS_PARAM_INT 0
#define TDS_PARAM_STRING 1
#define TDS_PARAM_FLOAT 2

struct tds_object {
	struct tds_sprite* sprite_handle;
	const char* type_name;

	int visible, layer, save; /* Save : will the object be exported? If not, the editor will not create a selector for it and the engine will ignore it during saving. */
	float x, y, z, angle, r, g, b, a, xspeed, yspeed;
	float cbox_width, cbox_height;

	tds_clock_point anim_lastframe;
	double anim_speed_offset;
	int anim_oneshot, anim_running;
	unsigned int current_frame;

	mat4x4 transform;

	void (*func_init)(struct tds_object* ptr);
	void (*func_destroy)(struct tds_object* ptr);
	void (*func_update)(struct tds_object* this);
	void (*func_draw)(struct tds_object* this);
	void (*func_msg)(struct tds_object* ptr, struct tds_object* from, int msg, void* param);

	void (*func_import)(struct tds_object* ptr, struct tds_object_param* param_list);
	struct tds_object_param* (*func_export)(struct tds_object* ptr);

	void* object_data; /* set to a custom structure per object type. */
	int object_handle;

	struct tds_handle_manager* hmgr;
	struct tds_sprite_cache* smgr;
	struct tds_sound_source* snd_src;

	float snd_volume;
	int snd_loop;
};

struct tds_object_type {
	const char* type_name;
	const char* default_sprite;
	struct tds_object_param* default_params; /* default_params should be used as a contiguous ARRAY, not a linked list! */
	int default_params_size;
	/* This allows for really simple static allocation. */

	int data_size, save;

	void (*func_init)(struct tds_object* ptr);
	void (*func_destroy)(struct tds_object* ptr);
	void (*func_update)(struct tds_object* ptr);
	void (*func_draw)(struct tds_object* ptr);
	void (*func_msg)(struct tds_object* ptr, struct tds_object* from, int msg, void* param);

	void (*func_import)(struct tds_object* ptr, struct tds_object_param* param_list);
	struct tds_object_param* (*func_export)(struct tds_object* ptr); /* Dynamically allocate the LL of object params, the runtime will release it */
};

struct tds_object_param {
	char key[32];
	char value[32];
	int type;

	struct tds_object_param* next;
};

struct tds_object* tds_object_create(struct tds_object_type* type, struct tds_handle_manager* hmgr, struct tds_sprite_cache* smgr, float x, float y, float z, struct tds_object_param* param_list);
void tds_object_free(struct tds_object* ptr);

void tds_object_set_sprite(struct tds_object* ptr, struct tds_sprite* sprite);
void tds_object_send_msg(struct tds_object* ptr, int handle, int msg, void* data);
vec4* tds_object_get_transform(struct tds_object* ptr);

void tds_object_anim_update(struct tds_object* ptr);
void tds_object_anim_start(struct tds_object* ptr);
void tds_object_anim_pause(struct tds_object* ptr);

void tds_object_update(struct tds_object* ptr);
void tds_object_init(struct tds_object* ptr);
void tds_object_draw(struct tds_object* ptr);
void tds_object_msg(struct tds_object* ptr, struct tds_object* sender, int msg, void* p);
void tds_object_destroy(struct tds_object* ptr);

void tds_object_import(struct tds_object* ptr, struct tds_object_param* param_list);
struct tds_object_param* tds_object_export(struct tds_object* ptr);

void tds_object_update_sndsrc(struct tds_object* ptr); /* The sound source needs to be updated with pos, vel, etc. */

int tds_object_anim_oneshot_finished(struct tds_object* ptr);

/* instead of encapsulating object functions (via more functions), systems and the like should just directly call the function pointers. */
/* they should be safe to call, they are checked by the creation functions */
