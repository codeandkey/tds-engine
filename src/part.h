#pragma once

/*
 * TDS particle subsystem
 * the previous "effect" subsystem animated particles via a CPU loop and rendered them as individual tris through a shader
 * this allowed for easier control over the animation of particles, but as most particles behave similarly it is a waste of cpu cycles.
 * also, rendering each particle as a tri hits performance pretty badly.
 * this particle subsystem efficiently organizes particles into types and allows for easy particle emission without the need of game-space subystems (EG effect_rain requires logic for all rain, as well as requiring a push to the TDS effect subsystem)
 * this particle subsystem also utilizes GLSL geometry shading for both animation and triangulation/rendering. MUCH nicer on performance.
 *
 * Each particle type can define the particle's default speed and direction, along with variance (random) vectors
 */

#define TDS_PART_TYPE_BLENDING_NORMAL 0
#define TDS_PART_TYPE_BLENDING_ADDITIVE 1

#define TDS_PART_MANAGER_SHADER_RENDER_VS "res/shaders/world_vs.glsl"
#define TDS_PART_MANAGER_SHADER_RENDER_GS "res/shaders/part_render_gs.glsl"
#define TDS_PART_MANAGER_SHADER_RENDER_FS "res/shaders/world_fs.glsl"
#define TDS_PART_MANAGER_SHADER_ANIMATE_VS "res/shaders/part_anim_vs.glsl"
#define TDS_PART_MANAGER_SHADER_ANIMATE_GS NULL
#define TDS_PART_MANAGER_SHADER_ANIMATE_FS NULL

#include "shader.h"

struct tds_part_type {
	const char* name;
	int max_particles;
	float position_offset[2], position_variance[2];
	float color_offset[4], color_variance[4];
	float speed_offset[2], speed_variance[2];
	int blending;
	float gravity;
	float a_decay; /* alpha decay is a simple linear slope. */
	float width, height;
	unsigned int rand_precision;
	struct tds_texture* tex;
};

struct tds_part_vertex {
	float x, y, r, g, b, a, dx, dy;
};

struct tds_part_system {
	/* a particle system manages the buffers of exactly one type of particle. systems are responsible for nothing other than cycling which particles are alive and which particles arent. */
	struct tds_part_type type;
	unsigned int vbo, vao;
	int write_offset;
	struct tds_part_system* next; /* used for tracking in part_manager */
};

struct tds_part_system* tds_part_system_create(struct tds_part_type type);
void tds_part_system_free(struct tds_part_system* ptr);

void tds_part_system_emit(struct tds_part_system* ptr, int num, float x, float y, float* color, float* speed);
void tds_part_system_flush(struct tds_part_system* ptr);

struct tds_part_manager {
	struct tds_part_system* systems;
	struct tds_shader* shader_part_render, *shader_part_animate;
};

struct tds_part_manager* tds_part_manager_create(void);
void tds_part_manager_free(struct tds_part_manager* ptr);

void tds_part_manager_add(struct tds_part_manager* ptr, struct tds_part_system* system);
struct tds_part_system* tds_part_manager_get_system(struct tds_part_manager* ptr, const char* name);

void tds_part_manager_flush(struct tds_part_manager* ptr);

void tds_part_manager_render(struct tds_part_manager* ptr);
void tds_part_manager_animate(struct tds_part_manager* ptr);
