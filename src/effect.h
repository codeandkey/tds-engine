#pragma once

/*
 * TDS effect subsystem
 * Effects are simply specialized rendering processes between the background and the world (rain, snow, etc)
 * To increase rendering speed this is NOT left to the game through the ECS -- it is implemented natively with structures via callbacks.
 */

/*
 * Two types of userdata are given to the effect type -- per-particle and per-system userdata.
 */

struct tds_effect_particle_part {
	float x, y, r, g, b, a;
};

struct tds_effect_particle_state {
	struct tds_texture* tex;
	struct tds_vertex_buffer* vb;
	void* data;
	int part_count;
	struct tds_effect_particle_part* part_buf;
};

typedef struct tds_effect_particle_state (*tds_effect_init_callback)(void);
typedef void (*tds_effect_free_callback)(struct tds_effect_particle_state* state);
typedef void (*tds_effect_update_callback)(struct tds_effect_particle_state* state);
typedef void (*tds_effect_render_callback)(struct tds_effect_particle_state* state);

struct tds_effect_type {
	const char* name;
	tds_effect_init_callback func_init;
	tds_effect_update_callback func_update;
	tds_effect_render_callback func_render;
	tds_effect_free_callback func_free;
};

struct tds_effect_instance {
	struct tds_effect_type* type;
	struct tds_effect_instance* next;
	struct tds_effect_particle_state state;
};

struct tds_effect {
	struct tds_effect_instance* list;
};

struct tds_effect* tds_effect_create(void);
void tds_effect_free(struct tds_effect* ptr);

void tds_effect_push(struct tds_effect* ptr, struct tds_effect_type* type);
void tds_effect_flush(struct tds_effect* ptr);

void tds_effect_update(struct tds_effect* ptr);
void tds_effect_render(struct tds_effect* ptr, unsigned int u_transform, unsigned int u_color);
