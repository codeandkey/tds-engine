#pragma once

/* The TDS rendering subsystem accepts a buffer of game objects, a game camera, and renders objects onto the screen. */
/* The render subsystem gets one shader, exclusively. No more structs acting as shader wrappers! */

#include "handle.h"
#include "camera.h"
#include "text.h"
#include "world.h"
#include "rt.h"

#define TDS_RENDER_SHADER_WORLD_VS "res/shaders/world_vs.glsl"
#define TDS_RENDER_SHADER_WORLD_FS "res/shaders/world_fs.glsl"
#define TDS_RENDER_SHADER_POINT_GS "res/shaders/point_gs.glsl"
#define TDS_RENDER_SHADER_POINT_FS "res/shaders/point_fs.glsl"
#define TDS_RENDER_SHADER_DIR_GS "res/shaders/dir_gs.glsl"
#define TDS_RENDER_SHADER_DIR_FS "res/shaders/dir_fs.glsl"
#define TDS_RENDER_SHADER_RECOMB_FS_POINT "res/shaders/recomb_fs_point.glsl"
#define TDS_RENDER_SHADER_RECOMB_FS_DIR "res/shaders/recomb_fs_dir.glsl"

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
	struct tds_text* text_handle;
	struct tds_rt* lightmap_rt, *point_rt, *dir_rt;

	struct tds_render_light* light_list;

	unsigned int render_vs, render_fs, render_program;
	unsigned int render_pgs, render_pfs, render_dgs, render_dfs, render_program_point, render_program_dir;
	unsigned int render_rfsp, render_program_recomb_point;
	unsigned int render_rdfs, render_rpfs;
	unsigned int render_rfsd, render_program_recomb_dir;

	int uniform_texture, uniform_color, uniform_transform;

	int p_uniform_texture, p_uniform_color, p_uniform_transform; /* Each lightmap rendering shader has their own set of uniform locations. */
	int d_uniform_texture, d_uniform_color, d_uniform_transform, d_uniform_dir;
	int rp_uniform_texture, rp_uniform_color, rp_uniform_transform;
	int rd_uniform_texture, rd_uniform_color, rd_uniform_transform;
};

struct tds_render* tds_render_create(struct tds_camera* camera, struct tds_handle_manager* hmgr, struct tds_text* text);
void tds_render_free(struct tds_render* ptr);

void tds_render_clear(struct tds_render* ptr);
void tds_render_draw(struct tds_render* ptr, struct tds_world* world);

void tds_render_submit_light(struct tds_render* ptr, struct tds_render_light lt);
void tds_render_clear_lights(struct tds_render* ptr);
