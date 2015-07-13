#pragma once

/* The TDS engine subsystem manages the main loop, threading, and all subsystems. */

#include "display.h"
#include "texture_cache.h"
#include "sprite_cache.h"
#include "render.h"

struct tds_engine_desc {
	const char* config_filename;
	const char* map_filename;
};

struct tds_engine_state {
	char* mapname;
	float fps;
	int entity_count;
};

struct tds_engine {
	struct tds_engine_desc desc;
	struct tds_engine_state state;

	struct tds_display* display_handle;
	struct tds_render* render_handle;
	struct tds_camera* camera_handle;
	struct tds_texture_cache* tc_handle;
	struct tds_sprite_cache* sc_handle;
	struct tds_handle_manager* object_buffer;
};

struct tds_engine* tds_engine_create(struct tds_engine_desc desc);
void tds_engine_free(struct tds_engine* ptr);

void tds_engine_run(struct tds_engine* ptr);
void tds_engine_flush_objects(struct tds_engine* ptr); /* destroys all objects in the buffer. */

struct tds_object* tds_engine_get_object_by_type(struct tds_engine* ptr, const char* typename);

struct tds_engine* tds_engine_global;
