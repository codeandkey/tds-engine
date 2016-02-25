#pragma once

/* The TDS engine subsystem manages the main loop, threading, and all subsystems. */

#include "display.h"
#include "texture_cache.h"
#include "sprite_cache.h"
#include "sound_cache.h"
#include "object_type_cache.h"
#include "sound_manager.h"
#include "console.h"
#include "render.h"
#include "key_map.h"
#include "input.h"
#include "input_map.h"
#include "block_map.h"
#include "world.h"
#include "signal.h"
#include "savestate.h"
#include "overlay.h"
#include "bg.h"
#include "profile.h"

#define TDS_MAP_PREFIX "res/maps/"

#define TDS_LOAD_BUFFER_SIZE 2048
#define TDS_LOAD_ATTR_SIZE 64
#define TDS_LOAD_WORLD_SIZE 4

/* TDS engine map spec :
 *
 * Maps in TDS are saved in JSON format to allow for easy structuring and editing.
 *
 * The root element should have an "objects" member; an array of map objects
 * The root element should also have a "settings" member; a table of map parameters.
 * 
 * Each object in the "objects" list should have the following values:
 *
 * x : X position
 * y : Y position
 * xspeed : X speed
 * yspeed : Y speed
 * angle : object angle
 * sprite_name : object sprite name
 * type_name : object type name
 * params : extra type-specific object parameters, passed to the import() type function
 */

struct tds_engine_desc {
	const char* config_filename;
	const char* map_filename;
	struct tds_key_map_template* game_input;
	int game_input_size;
	unsigned int save_index;

	void (*func_load_sounds)(struct tds_sound_cache* sndc_handle);
	void (*func_load_sprites)(struct tds_sprite_cache* sc_handle, struct tds_texture_cache* tc_handle);
	void (*func_load_object_types)(struct tds_object_type_cache* otc_handle);
	void (*func_load_block_map)(struct tds_block_map* block_map_handle, struct tds_texture_cache* tc_handle);
};

struct tds_engine_state {
	float fps;
	int entity_maxindex;
};

struct tds_engine_object_list {
	struct tds_object** buffer;
	int size;
};

struct tds_engine {
	struct tds_engine_desc desc;
	struct tds_engine_state state;

	struct tds_display* display_handle;
	struct tds_render* render_handle;
	struct tds_camera* camera_handle;
	struct tds_texture_cache* tc_handle;
	struct tds_sprite_cache* sc_handle;
	struct tds_sound_cache* sndc_handle;
	struct tds_object_type_cache* otc_handle;
	struct tds_handle_manager* object_buffer;
	struct tds_input* input_handle;
	struct tds_input_map* input_map_handle;
	struct tds_key_map* key_map_handle;
	struct tds_sound_manager* sound_manager_handle;
	struct tds_console* console_handle;
	struct tds_block_map* block_map_handle;
	struct tds_world* world_handle;
	struct tds_savestate* savestate_handle;
	struct tds_overlay* overlay_handle;
	struct tds_bg* bg_handle;
	struct tds_profile* profile_handle;

	int run_flag;
	struct tds_object** object_list;

	int enable_update, enable_draw;
};

struct tds_engine* tds_engine_create(struct tds_engine_desc desc);
void tds_engine_free(struct tds_engine* ptr);

void tds_engine_run(struct tds_engine* ptr);
void tds_engine_flush_objects(struct tds_engine* ptr); /* destroys all objects in the buffer. */
void tds_engine_terminate(struct tds_engine* ptr); /* flags the engine to stop soon */

struct tds_object* tds_engine_get_object_by_type(struct tds_engine* ptr, const char* type);
struct tds_engine_object_list tds_engine_get_object_list_by_type(struct tds_engine* ptr, const char* type); /* Allocates a buffer. Freed by the engine on shutdown. */

void tds_engine_load(struct tds_engine* ptr, const char* mapname);
void tds_engine_save(struct tds_engine* ptr, const char* mapname);

void tds_engine_destroy_objects(struct tds_engine* ptr, const char* type_name);
void tds_engine_broadcast(struct tds_engine* ptr, int msg, void* param);

extern struct tds_engine* tds_engine_global;
