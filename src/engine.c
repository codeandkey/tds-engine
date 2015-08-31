#include "engine.h"
#include "config.h"
#include "display.h"
#include "object.h"
#include "handle.h"
#include "memory.h"
#include "clock.h"
#include "sprite.h"
#include "sound_buffer.h"
#include "log.h"

#include "objects/all.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TDS_ENGINE_TIMESTEP 144.0f

struct tds_engine* tds_engine_global = NULL;

static void _tds_engine_load_sprites(struct tds_engine* ptr);
static void _tds_engine_load_sounds(struct tds_engine* ptr);

struct tds_engine* tds_engine_create(struct tds_engine_desc desc) {
	if (tds_engine_global) {
		tds_logf(TDS_LOG_CRITICAL, "Only one engine can exist!\n");
		return NULL;
	}

	struct tds_engine* output = tds_malloc(sizeof(struct tds_engine));

	tds_engine_global = output;

	struct tds_display_desc display_desc;
	struct tds_config* conf;

	output->desc = desc;
	output->object_list = NULL;

	output->state.mapname = (char*) desc.map_filename;
	output->state.fps = 0.0f;
	output->state.entity_count = 0;

	tds_logf(TDS_LOG_MESSAGE, "Initializing TDS engine..\n");

	/* Config loading */
	conf = tds_config_create(desc.config_filename);

	tds_logf(TDS_LOG_MESSAGE, "Loaded engine configuration.\n");

	/* Display subsystem */
	display_desc.width = tds_config_get_int(conf, "width");
	display_desc.height = tds_config_get_int(conf, "height");
	display_desc.fs = tds_config_get_int(conf, "fullscreen");
	display_desc.vsync = tds_config_get_int(conf, "vsync");
	display_desc.msaa = tds_config_get_int(conf, "msaa");

	tds_logf(TDS_LOG_MESSAGE, "Loaded display description. Video mode : %d by %d, FS %s, VSYNC %s, MSAA %d\n", display_desc.width, display_desc.height, display_desc.fs ? "on" : "off", display_desc.vsync ? "on" : "off", display_desc.msaa);
	output->display_handle = tds_display_create(display_desc);
	tds_logf(TDS_LOG_MESSAGE, "Created display.\n");
	/* End display subsystem */

	output->tc_handle = tds_texture_cache_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized texture cache.\n");

	output->sc_handle = tds_sprite_cache_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized sprite cache.\n");

	output->sndc_handle = tds_sound_cache_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialize sound cache.\n");

	output->object_buffer = tds_handle_manager_create(1024);
	tds_logf(TDS_LOG_MESSAGE, "Initialized object buffer.\n");

	output->camera_handle = tds_camera_create(output->display_handle);
	tds_camera_set(output->camera_handle, 10.0f, 0.0f, 0.0f);
	tds_logf(TDS_LOG_MESSAGE, "Initialized camera system.\n");

	output->text_handle = tds_text_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized text system.\n");

	output->render_handle = tds_render_create(output->camera_handle, output->object_buffer, output->text_handle);
	tds_logf(TDS_LOG_MESSAGE, "Initialized render system.\n");

	output->input_handle = tds_input_create(output->display_handle);
	tds_logf(TDS_LOG_MESSAGE, "Initialized input system.\n");

	output->sound_manager_handle = tds_sound_manager_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized OpenAL context.\n");

	output->input_map_handle = tds_input_map_create(output->input_handle);
	tds_logf(TDS_LOG_MESSAGE, "Initialized input mapping system.\n");

	output->key_map_handle = tds_key_map_create(desc.game_input, desc.game_input_size);
	tds_logf(TDS_LOG_MESSAGE, "Initialized key mapping system.\n");

	_tds_engine_load_sprites(output);
	tds_logf(TDS_LOG_MESSAGE, "Loaded sprites.\n");

	_tds_engine_load_sounds(output);
	tds_logf(TDS_LOG_MESSAGE, "Loaded sounds.\n");

	/* Free configs */
	tds_config_free(conf);

	tds_logf(TDS_LOG_MESSAGE, "Done initializing everything.\n");
	return output;
}

void tds_engine_free(struct tds_engine* ptr) {
	tds_logf(TDS_LOG_MESSAGE, "Freeing engine structure and subsystems.\n");

	if (ptr->object_list) {
		tds_free(ptr->object_list);
	}

	tds_engine_flush_objects(ptr);

	tds_text_free(ptr->text_handle);
	tds_input_free(ptr->input_handle);
	tds_input_map_free(ptr->input_map_handle);
	tds_key_map_free(ptr->key_map_handle);
	tds_render_free(ptr->render_handle);
	tds_camera_free(ptr->camera_handle);
	tds_display_free(ptr->display_handle);
	tds_texture_cache_free(ptr->tc_handle);
	tds_sprite_cache_free(ptr->sc_handle);
	tds_sound_cache_free(ptr->sndc_handle);
	tds_sound_manager_free(ptr->sound_manager_handle);
	tds_handle_manager_free(ptr->object_buffer);
	tds_free(ptr);
}

void tds_engine_run(struct tds_engine* ptr) {
	int running = ptr->run_flag = 1;

	tds_logf(TDS_LOG_MESSAGE, "Starting engine mainloop.\n");

	{
		/* Test code to do stuff. */
		tds_object_create(&tds_obj_system_type, ptr->object_buffer, ptr->sc_handle, 0.0f, 0.0f, 0.0f, NULL);
		tds_object_create(&tds_obj_player_type, ptr->object_buffer, ptr->sc_handle, 0.0f, 0.0f, 0.0f, NULL);
		tds_object_create(&tds_obj_enemy_basic_type, ptr->object_buffer, ptr->sc_handle, -1.0f, 0.0f, 0.0f, NULL)->angle = 3.141f;
		tds_object_create(&tds_obj_player_legs_type, ptr->object_buffer, ptr->sc_handle, 0.0f, 0.0f, 0.0f, NULL);
		tds_object_create(&tds_obj_player_arms_type, ptr->object_buffer, ptr->sc_handle, 0.0f, 0.0f, 0.0f, NULL);
		tds_object_create(&tds_obj_cursor_type, ptr->object_buffer, ptr->sc_handle, 0.0f, 0.0f, 0.0f, NULL);
		tds_object_create(&tds_obj_wall_type, ptr->object_buffer, ptr->sc_handle, 3.0f, 0.0f, 0.0f, NULL);

		/* Not to be in final game. */
	}

	tds_sound_manager_set_pos(ptr->sound_manager_handle, ptr->camera_handle->x, ptr->camera_handle->y);

	/* The game, like 'hunter' will use an accumulator-based approach with a fixed timestep.
	 * to support higher-refresh rate monitors, the timestep will be set to 144hz. */

	/* The game logic will update on a time-dependent basis, but the game will draw whenever possible. */

	tds_clock_point dt_point = tds_clock_get_point();
	double accumulator = 0.0f;
	double timestep_ms = 1000.0f / (double) TDS_ENGINE_TIMESTEP;

	while (running && ptr->run_flag) {
		running &= !tds_display_get_close(ptr->display_handle);

		double delta_ms = tds_clock_get_ms(dt_point);
		dt_point = tds_clock_get_point();
		accumulator += delta_ms;

		/* We approximate the fps using the delta frame time. */
		ptr->state.fps = 1000.0f / delta_ms;

		// tds_logf(TDS_LOG_MESSAGE, "frame : accum = %f ms, delta_ms = %f ms, timestep = %f\n", accumulator, delta_ms, timestep_ms);

		tds_display_update(ptr->display_handle);
		tds_input_update(ptr->input_handle);

		while (accumulator >= timestep_ms) {
			accumulator -= timestep_ms;

			/* Run game update logic. */

			for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
				struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[i].data;

				if (!target) {
					continue;
				}

				tds_object_update(target);
			}
		}

		/* Run game draw logic. */
		tds_render_clear(ptr->render_handle); /* We clear before executing the draw functions, otherwise the text buffer would be destroyed */

		for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
			struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[i].data;

			if (!target) {
				continue;
			}

			tds_object_draw(target);
		}

		tds_render_draw(ptr->render_handle);
		tds_display_swap(ptr->display_handle);
	}

	tds_logf(TDS_LOG_MESSAGE, "Finished engine mainloop.\n");
}

void tds_engine_flush_objects(struct tds_engine* ptr) {
	for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
		if (ptr->object_buffer->buffer[i].data) {
			tds_object_free(ptr->object_buffer->buffer[i].data);
		}
	}
}

struct tds_object* tds_engine_get_object_by_type(struct tds_engine* ptr, const char* typename) {
	for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
		if (!ptr->object_buffer->buffer[i].data) {
			continue;
		}

		if (!strcmp(((struct tds_object*) ptr->object_buffer->buffer[i].data)->type_name, typename)) {
			return ptr->object_buffer->buffer[i].data;
		}
	}

	return NULL;
}

struct tds_engine_object_list tds_engine_get_object_list_by_type(struct tds_engine* ptr, const char* typename) {
	if (ptr->object_list) {
		tds_free(ptr->object_list);
		ptr->object_list = NULL;
	}

	struct tds_engine_object_list output;

	output.size = 0;

	for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
		if (!ptr->object_buffer->buffer[i].data) {
			continue;
		}

		if (!strcmp(((struct tds_object*) ptr->object_buffer->buffer[i].data)->type_name, typename)) {
			ptr->object_list = tds_realloc(ptr->object_list, sizeof(struct tds_object*) * ++output.size);
			ptr->object_list[output.size - 1] = (struct tds_object*) ptr->object_buffer->buffer[i].data;
		}
	}

	output.buffer = ptr->object_list;
	return output;
}

void tds_engine_terminate(struct tds_engine* ptr) {
	ptr->run_flag = 0;
}

/* The TDS map format stores each entity and all entity data. */
/* Entity data saved (in bin format) : 
 *
 * position, float2
 * velocity, float2
 * angle, float
 * sprite cache index, int+char[]
 * current animation frame, int
 * entity type data, int+char[]
 * entity type name, int+char[]
 */

void tds_engine_load_map(struct tds_engine* ptr, char* mapname) {
	tds_engine_flush_objects(ptr);

	FILE* ifile = fopen(mapname, "rb");

	if (!ifile) {
		tds_logf(TDS_LOG_WARNING, "Failed to open map %s for reading.\n", mapname);
		return;
	}

	while (!feof(ifile)) {
		float e_floats[5];

		if (fread(e_floats, sizeof(float), 5, ifile) != 5) {
			tds_logf(TDS_LOG_CRITICAL, "Format error in entity positional data.\n");
			break;
		}

		int sprite_name_len, data_len, typename_len, current_frame;
		char* sprite_name, *data, *typename;

		fread(&sprite_name_len, sizeof(int), 1, ifile);
		sprite_name = tds_malloc(sprite_name_len + 1);
		fread(sprite_name, 1, sprite_name_len, ifile);
		sprite_name[sprite_name_len] = 0;

		fread(&current_frame, sizeof(int), 1, ifile);

		fread(&data_len, sizeof(int), 1, ifile);
		data = tds_malloc(data_len + 1);
		fread(data, 1, data_len, ifile);
		data[data_len] = 0;

		fread(&typename_len, sizeof(int), 1, ifile);
		typename = tds_malloc(typename_len + 1);
		fread(typename, 1, typename_len, ifile);
		typename[typename_len] = 0;

		struct tds_object* new_object = tds_object_create(tds_object_type_get_by_name((const char*) typename), ptr->object_buffer, ptr->sc_handle, e_floats[0], e_floats[1], 0.0f, data);

		new_object->xspeed = e_floats[2];
		new_object->yspeed = e_floats[3];
		new_object->angle = e_floats[4];
	}

	fclose(ifile);
}

void tds_engine_save_map(struct tds_engine* ptr, char* mapname) {	
}

void _tds_engine_load_sprites(struct tds_engine* ptr) {
	tds_sprite_cache_add(ptr->sc_handle, "player", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/player.png", 32, 32), 1.0f, 1.0f, 80.0f));
	tds_sprite_cache_add(ptr->sc_handle, "enemy_basic", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/enemy.png", 32, 32), 1.0f, 1.0f, 80.0f));
	tds_sprite_cache_add(ptr->sc_handle, "enemy_basic_aim", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/enemy_aim.png", 64, 64), 2.0f, 2.0f, 80.0f));
	tds_sprite_cache_add(ptr->sc_handle, "player_body_swing", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/player_swing_body.png", 96, 96), 3.0f, 3.0f, 40.0f));
	tds_sprite_cache_add(ptr->sc_handle, "player_legs", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/player_legs.png", 32, 32), 1.0f, 1.0f, 80.0f));
	tds_sprite_cache_add(ptr->sc_handle, "player_arms_katana", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/player_swing_arms.png", 96, 96), 3.0f, 3.0f, 40.0f));
	tds_sprite_cache_add(ptr->sc_handle, "wall_concrete_medium", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/wall_concrete.png", 32, 32), 1.0f, 1.0f, 0.0f));
	tds_sprite_cache_add(ptr->sc_handle, "cursor", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/cursor.png", 32, 32), 0.3f, 0.3f, 0.0f));
	tds_sprite_cache_add(ptr->sc_handle, "bullet", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/sprites/bullet.png", 32, 32), 0.4f, 0.25f, 0.0f));
	tds_sprite_cache_add(ptr->sc_handle, "font_debug", tds_sprite_create(tds_texture_cache_get(ptr->tc_handle, "res/fonts/debug.png", 8, 8), 0.25f, 0.25f, 0.0f));
}

void _tds_engine_load_sounds(struct tds_engine* ptr) {
	tds_sound_cache_add(ptr->sndc_handle, "music_bg", tds_sound_buffer_create("res/music/music_bg.ogg"));
	tds_sound_cache_add(ptr->sndc_handle, "sound_swish", tds_sound_buffer_create("res/sounds/swish.ogg"));
	tds_sound_cache_add(ptr->sndc_handle, "sound_pistol", tds_sound_buffer_create("res/sounds/pistol.ogg"));
}
