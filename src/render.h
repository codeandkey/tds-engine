#pragma once

/* The TDS rendering subsystem accepts a buffer of game objects, a game camera, and renders objects onto the screen. */
/* The render subsystem gets one shader, exclusively. No more structs acting as shader wrappers! */

#include "handle.h"
#include "camera.h"
#include "world.h"
#include "rt.h"
#include "shader.h"
#include "render_flat.h"

#define TDS_RENDER_SHADER_WORLD_VS "res/shaders/world_vs.glsl"
#define TDS_RENDER_SHADER_WORLD_FS "res/shaders/world_fs.glsl"
#define TDS_RENDER_SHADER_POINT_GS "res/shaders/point_gs.glsl"
#define TDS_RENDER_SHADER_POINT_FS "res/shaders/point_fs.glsl"
#define TDS_RENDER_SHADER_DIR_GS "res/shaders/dir_gs.glsl"
#define TDS_RENDER_SHADER_DIR_FS "res/shaders/dir_fs.glsl"
#define TDS_RENDER_SHADER_RECOMB_FS_POINT "res/shaders/recomb_fs_point.glsl"
#define TDS_RENDER_SHADER_RECOMB_FS_DIR "res/shaders/recomb_fs_dir.glsl"
#define TDS_RENDER_SHADER_HBLUR_VS "res/shaders/hblur_vs.glsl"
#define TDS_RENDER_SHADER_VBLUR_VS "res/shaders/vblur_vs.glsl"
#define TDS_RENDER_SHADER_BLUR_FS "res/shaders/blur_fs.glsl"
#define TDS_RENDER_SHADER_BLOOM_FS "res/shaders/bloom_fs.glsl"

#define TDS_RENDER_LIGHT_POINT 0
#define TDS_RENDER_LIGHT_DIRECTIONAL 1

#define TDS_RENDER_POINT_RT_SIZE 2048

struct tds_render_light {
	unsigned int type;
	float x, y, r, g, b, dist;
	struct tds_render_light* next;
};

struct tds_render {
	struct tds_camera* camera_handle;
	struct tds_handle_manager* object_buffer;
	struct tds_rt* lightmap_rt, *point_rt, *dir_rt, *post_rt1, *post_rt2, *post_rt3;
	struct tds_render_light* light_list;

	struct tds_shader* shader_passthrough;
	struct tds_shader* shader_light_point;
	struct tds_shader* shader_light_dir;
	struct tds_shader* shader_recomb_point;
	struct tds_shader* shader_recomb_dir;
	struct tds_shader* shader_bloom;
	struct tds_shader* shader_hblur;
	struct tds_shader* shader_vblur;

	unsigned int enable_bloom, enable_dynlights;
	int enable_wireframe, enable_aabb;

	float ambient_r, ambient_b, ambient_g;
};

struct tds_render* tds_render_create(struct tds_camera* camera, struct tds_handle_manager* hmgr);
void tds_render_free(struct tds_render* ptr);

void tds_render_clear(struct tds_render* ptr);
void tds_render_draw(struct tds_render* ptr, struct tds_world** world_buffer, int world_count, struct tds_render_flat* flat_world, struct tds_render_flat* flat_overlay);

void tds_render_submit_light(struct tds_render* ptr, struct tds_render_light lt);
void tds_render_clear_lights(struct tds_render* ptr);

void tds_render_set_ambient_brightness(struct tds_render* ptr, float brightness);
