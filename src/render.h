#pragma once

/* The TDS rendering subsystem accepts a buffer of game objects, a game camera, and renders objects onto the screen. */
/* The render subsystem gets one shader, exclusively. No more structs acting as shader wrappers! */

#include "handle.h"
#include "camera.h"
#include "text.h"

#define TDS_RENDER_SHADER_WORLD_VS "res/shaders/world_vs.glsl"
#define TDS_RENDER_SHADER_WORLD_FS "res/shaders/world_fs.glsl"

struct tds_render {
	struct tds_camera* camera_handle;
	struct tds_handle_manager* object_buffer;
	struct tds_text* text_handle;

	unsigned int render_vs, render_fs, render_program;
	int uniform_texture, uniform_color, uniform_transform;
};

struct tds_render* tds_render_create(struct tds_camera* camera, struct tds_handle_manager* hmgr, struct tds_text* text);
void tds_render_free(struct tds_render* ptr);

void tds_render_clear(struct tds_render* ptr);
void tds_render_draw(struct tds_render* ptr);
