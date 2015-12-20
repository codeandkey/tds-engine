#pragma once

#include "sprite.h"
#include "handle.h"
#include "linmath.h"
#include "clock.h"
#include "sprite_cache.h"
#include "sound_source.h"

struct tds_object_param;

#define TDS_PARAM_VALSIZE 32

#define TDS_PARAM_INT 0
#define TDS_PARAM_STRING 1
#define TDS_PARAM_FLOAT 2
#define TDS_PARAM_UINT 3

struct tds_object_param {
	unsigned int key;

	union {
		char spart[TDS_PARAM_VALSIZE];
		int ipart;
		unsigned int upart;
		float fpart;
	};

	int type;

	struct tds_object_param* next;
}; // To do this right, no object code should EVER have to deal with this structure.

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

	void* object_data; /* set to a custom structure per object type. */
	int object_handle;

	struct tds_handle_manager* hmgr;
	struct tds_sprite_cache* smgr;
	struct tds_sound_source* snd_src;

	float snd_volume;
	int snd_loop;

	struct tds_object_param* param_list;
};

struct tds_object_type {
	const char* type_name;
	const char* default_sprite;

	int data_size, save;

	void (*func_init)(struct tds_object* ptr);
	void (*func_destroy)(struct tds_object* ptr);
	void (*func_update)(struct tds_object* ptr);
	void (*func_draw)(struct tds_object* ptr);
	void (*func_msg)(struct tds_object* ptr, struct tds_object* from, int msg, void* param);
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

void tds_object_update_sndsrc(struct tds_object* ptr); /* The sound source needs to be updated with pos, vel, etc. */

int tds_object_anim_oneshot_finished(struct tds_object* ptr);

/* We will have a nice and memory-safe API for manipulating object parameters.
 * All memory will already be allocated and managed by the runtime. Objects are just passed pointers to the original data.
 * There are also setter functions for convienence.
 * The setter functions create the parameter if it did not exist before. */

int* tds_object_get_ipart(struct tds_object* ptr, unsigned int index);
char* tds_object_get_spart(struct tds_object* ptr, unsigned int index);
unsigned int* tds_object_get_upart(struct tds_object* ptr, unsigned int index);
float* tds_object_get_fpart(struct tds_object* ptr, unsigned int index);

void tds_object_set_ipart(struct tds_object* ptr, unsigned int index, int value);
void tds_object_set_spart(struct tds_object* ptr, unsigned int index, char* value, int value_size);
void tds_object_set_upart(struct tds_object* ptr, unsigned int index, unsigned int value);
void tds_object_set_fpart(struct tds_object* ptr, unsigned int index, float value);

void tds_object_unset(struct tds_object* ptr, unsigned int index);

/* Objects do not need an API for iterating parameters. Only the engine needs to do this, and it is done by managing the linked list directly. Do NOT do this from object code or stuff could get nasty. */
