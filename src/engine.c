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
#include "msg.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "objects/objects.h"

#define TDS_ENGINE_TIMESTEP 120.0f

struct tds_engine* tds_engine_global = NULL;

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

	output->enable_update = output->enable_draw = 1;

	output->state.fps = 0.0f;
	output->state.entity_maxindex = 0;

	tds_logf(TDS_LOG_MESSAGE, "Initializing TDS engine..\n");

	tds_signal_init();
	tds_logf(TDS_LOG_MESSAGE, "Registered signal handlers.\n");

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
	tds_logf(TDS_LOG_MESSAGE, "Initialized sound cache.\n");

	output->otc_handle = tds_object_type_cache_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized object type cache.\n");

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

	output->block_map_handle = tds_block_map_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized block mapping subsystem.\n");

	output->world_handle = tds_world_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized world subsystem.\n");

	output->savestate_handle = tds_savestate_create();
	tds_savestate_set_index(output->savestate_handle, desc.save_index);
	tds_logf(TDS_LOG_MESSAGE, "Initialized savestate subsystem.\n");

	if (desc.func_load_sprites) {
		desc.func_load_sprites(output->sc_handle, output->tc_handle);
		tds_logf(TDS_LOG_MESSAGE, "Loaded sprites.\n");
	}

	if (desc.func_load_block_map) {
		desc.func_load_block_map(output->block_map_handle, output->tc_handle);
		tds_logf(TDS_LOG_MESSAGE, "Loaded block types.\n");
	}

	if (desc.func_load_sounds) {
		desc.func_load_sounds(output->sndc_handle);
		tds_logf(TDS_LOG_MESSAGE, "Loaded sounds.\n");
	}

	/* Here, we add the editor objects. */
	tds_load_editor_objects(output->otc_handle);

	if (desc.func_load_object_types) {
		desc.func_load_object_types(output->otc_handle);
		tds_logf(TDS_LOG_MESSAGE, "Loaded object types.\n");
	}

	output->console_handle = tds_console_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized console.\n");

	/* Free configs */
	tds_config_free(conf);
	tds_logf(TDS_LOG_MESSAGE, "Done initializing everything.\n");
	tds_logf(TDS_LOG_MESSAGE, "Engine is ready to roll!\n");

	if (desc.map_filename) {
		tds_logf(TDS_LOG_MESSAGE, "Loading initial map [%s].\n", desc.map_filename);
		tds_engine_load(output, desc.map_filename);
	}

	return output;
}

void tds_engine_free(struct tds_engine* ptr) {
	tds_logf(TDS_LOG_MESSAGE, "Freeing engine structure and subsystems.\n");

	if (ptr->object_list) {
		tds_free(ptr->object_list);
	}

	tds_engine_flush_objects(ptr);

	tds_block_map_free(ptr->block_map_handle);
	tds_world_free(ptr->world_handle);
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
	tds_object_type_cache_free(ptr->otc_handle);
	tds_sound_manager_free(ptr->sound_manager_handle);
	tds_handle_manager_free(ptr->object_buffer);
	tds_console_free(ptr->console_handle);
	tds_savestate_free(ptr->savestate_handle);
	tds_free(ptr);
}

void tds_engine_run(struct tds_engine* ptr) {
	int running = ptr->run_flag = 1;

	tds_logf(TDS_LOG_MESSAGE, "Starting engine mainloop.\n");
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

		// Useful message for debugging frame delta timings.
		// tds_logf(TDS_LOG_MESSAGE, "frame : accum = %f ms, delta_ms = %f ms, timestep = %f\n", accumulator, delta_ms, timestep_ms);

		ptr->state.entity_maxindex = ptr->object_buffer->max_index;

		tds_display_update(ptr->display_handle);

		while (accumulator >= timestep_ms) {
			accumulator -= timestep_ms;

			/* Run game update logic. */
			/* Even if updating is disabled, we still want to run down the accumulator. */

			tds_input_update(ptr->input_handle);

			if (ptr->enable_update) {
				for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
					struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[i].data;

					if (!target) {
						continue;
					}

					tds_object_update(target);
				}
			}
		}

		/* Run game draw logic. */
		tds_render_clear(ptr->render_handle); /* We clear before executing the draw functions, otherwise the text buffer would be destroyed */

		if (ptr->enable_draw) {
			for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
				struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[i].data;

				if (!target) {
					continue;
				}

				tds_object_draw(target);
			}
		}

		tds_console_draw(ptr->console_handle);

		tds_render_draw(ptr->render_handle, ptr->world_handle);
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

void tds_engine_load(struct tds_engine* ptr, const char* mapname) {
	tds_engine_flush_objects(ptr);

	char* str_filename = tds_malloc(strlen(mapname) + strlen(TDS_MAP_PREFIX) + 1);

	memcpy(str_filename, TDS_MAP_PREFIX, strlen(TDS_MAP_PREFIX));
	memcpy(str_filename + strlen(TDS_MAP_PREFIX), mapname, strlen(mapname));

	str_filename[strlen(TDS_MAP_PREFIX) + strlen(mapname)] = 0;
	tds_logf(TDS_LOG_DEBUG, "Loading map [%s]\n", str_filename);

	FILE* fd_input = fopen(str_filename, "rb");

	if (!fd_input) {
		tds_logf(TDS_LOG_CRITICAL, "Loading failed : the file could not be opened.\n");
		return;
	}

	uint32_t world_width, world_height;

	fread(&world_width, sizeof(world_width), 1, fd_input);
	fread(&world_height, sizeof(world_height), 1, fd_input);

	if (world_width && world_height) {
		uint8_t* block_buffer = tds_malloc(sizeof(*block_buffer) * world_width * world_height);

		for (int i = 0; i < world_height * world_width; ++i) {
			fread(block_buffer + i, sizeof *block_buffer, 1, fd_input);
		}

		tds_world_load(ptr->world_handle, block_buffer, world_width, world_height);
		tds_free(block_buffer);
	}

	while(!feof(fd_input)) {
		float x, y, dx, dy, angle;
		int type_size, param_count, result = 1;
		char* type_name;

		result &= fread(&x, sizeof(float), 1, fd_input);

		if (!result) {
			break;
		}

		result &= fread(&y, sizeof(float), 1, fd_input);

		result &= fread(&dx, sizeof(float), 1, fd_input);
		result &= fread(&dy, sizeof(float), 1, fd_input);
		result &= fread(&angle, sizeof(float), 1, fd_input);

		if (!result) {
			tds_logf(TDS_LOG_CRITICAL, "Loading failed : malformed header, could not finish read.\n");
			return;
		}

		tds_logf(TDS_LOG_DEBUG, "Header : x %f y %f dx %f dy %f angle %f\n", x, y, dx, dy, angle);

		result &= fread(&type_size, sizeof(int), 1, fd_input);
		tds_logf(TDS_LOG_DEBUG, "Typename size : %d\n", type_size);
		type_name = tds_malloc(type_size + 1);
		result &= fread(type_name, type_size, 1, fd_input);
		type_name[type_size] = 0;

		tds_logf(TDS_LOG_DEBUG, "Typename : [%s]\n", type_name);

		result &= fread(&param_count, sizeof(int), 1, fd_input);

		tds_logf(TDS_LOG_DEBUG, "Parameter count : %d\n", param_count);

		struct tds_object_param* param_list_head = NULL, *param_list_tail = NULL, *new_param = NULL;

		for (int i = 0; i < param_count; ++i) {
			new_param = tds_malloc(sizeof *new_param);

			int param_valsize, param_type;

			unsigned int param_key;

			fread(&param_key, sizeof param_key, 1, fd_input);
			fread(&param_type, sizeof param_type, 1, fd_input);
			fread(&param_valsize, sizeof param_valsize, 1, fd_input);

			tds_logf(TDS_LOG_DEBUG, "Reading param; key %d, type %d\n", param_key, param_type);

			switch (param_type) {
			case TDS_PARAM_INT:
				if (param_valsize != sizeof new_param->ipart) {
					tds_logf(TDS_LOG_CRITICAL, "Type size mismatch!\n");
				}
				fread(&new_param->ipart, param_valsize, 1, fd_input);
				break;
			case TDS_PARAM_UINT:
				if (param_valsize != sizeof new_param->upart) {
					tds_logf(TDS_LOG_CRITICAL, "Type size mismatch!\n");
				}
				fread(&new_param->upart, param_valsize, 1, fd_input);
				break;
			case TDS_PARAM_FLOAT:
				if (param_valsize != sizeof new_param->fpart) {
					tds_logf(TDS_LOG_CRITICAL, "Type size mismatch!\n");
				}
				fread(&new_param->fpart, param_valsize, 1, fd_input);
				break;
			case TDS_PARAM_STRING:
				if (param_valsize != sizeof new_param->spart / sizeof new_param->spart[0]) {
					tds_logf(TDS_LOG_CRITICAL, "Type size mismatch!\n");
				}
				fread(new_param->spart, param_valsize, 1, fd_input);
				break;
			}

			if (param_list_tail) {
				param_list_tail->next = new_param;
				param_list_tail = new_param;
			} else {
				param_list_tail = param_list_head = new_param;
			}
		}

		/* We have the object param list ready. */
		/* We try and retrieve the type information. */

		struct tds_object_type* type_ptr = tds_object_type_cache_get(ptr->otc_handle, type_name);
		struct tds_object* obj_new = tds_object_create(type_ptr, ptr->object_buffer, ptr->sc_handle, x, y, 0.0f, param_list_head);

		obj_new->angle = angle;
		tds_free(type_name);

		if (tds_editor_get_mode() == TDS_EDITOR_MODE_OBJECTS) {
			tds_editor_add_selector(obj_new);
		}
	}

	fclose(fd_input);
	tds_free(str_filename);

	tds_engine_broadcast(ptr, TDS_MSG_MAP_READY, 0);
}

void tds_engine_save(struct tds_engine* ptr, const char* mapname) {
	char* str_filename = tds_malloc(strlen(mapname) + strlen(TDS_MAP_PREFIX) + 1);

	memcpy(str_filename, TDS_MAP_PREFIX, strlen(TDS_MAP_PREFIX));
	memcpy(str_filename + strlen(TDS_MAP_PREFIX), mapname, strlen(mapname));

	str_filename[strlen(TDS_MAP_PREFIX) + strlen(mapname)] = 0;

	tds_logf(TDS_LOG_DEBUG, "Saving to map [%s]\n", str_filename);

	FILE* fd_output = fopen(str_filename, "wb");

	if (!fd_output) {
		tds_logf(TDS_LOG_CRITICAL, "Saving failed : could not open file\n");
		return;
	}

	/* We then serialize the world into a buffer and write it. */
	int world_width = ptr->world_handle->width, world_height = ptr->world_handle->height;
	uint8_t* block_buffer = tds_malloc(sizeof(*block_buffer) * world_width * world_height);

	tds_world_save(ptr->world_handle, block_buffer, world_width, world_height);

	fwrite(&world_width, sizeof world_width, 1, fd_output);
	fwrite(&world_height, sizeof world_height, 1, fd_output);

	fwrite(block_buffer, sizeof(*block_buffer) * world_width * world_height, 1, fd_output);
	tds_free(block_buffer);

	/* To serialize, we must iterate through each object and save entity info + type params. */

	for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
		/* We want to save position, angle, spritename, typename, etc.. */
		/* We also want to store object type parameters to be passed to/from the import/export functions. */

		struct tds_object* target = ptr->object_buffer->buffer[i].data;

		if (!target) {
			continue;
		}

		if (!target->save) {
			continue;
		}

		/* First save primitiva values, then move through the parameter list. */

		int type_size = strlen(target->type_name);

		fwrite(&target->x, sizeof(float), 1, fd_output);
		fwrite(&target->y, sizeof(float), 1, fd_output);
		fwrite(&target->xspeed, sizeof(float), 1, fd_output);
		fwrite(&target->yspeed, sizeof(float), 1, fd_output);
		fwrite(&target->angle, sizeof(float), 1, fd_output);
		fwrite(&type_size, sizeof(int), 1, fd_output);
		fwrite(target->type_name, type_size, 1, fd_output);

		struct tds_object_param* param_list_head = target->param_list, *current_param = target->param_list;
		int param_count = 0;

		/* We run through the list once to get the count, twice to save all of the params. */

		while (current_param) {
			param_count++;
			current_param = current_param->next;
		}

		fwrite(&param_count, sizeof(int), 1, fd_output);
		current_param = param_list_head;

		while (current_param) {
			int valsize = 0;

			switch (current_param->type) {
			case TDS_PARAM_INT:
				valsize = sizeof current_param->ipart;
				break;
			case TDS_PARAM_UINT:
				valsize = sizeof current_param->upart;
				break;
			case TDS_PARAM_FLOAT:
				valsize = sizeof current_param->fpart;
				break;
			case TDS_PARAM_STRING:
				valsize = sizeof current_param->spart / sizeof current_param->spart[0];
				break;
			}

			fwrite(&current_param->key, sizeof current_param->key, 1, fd_output);
			fwrite(&current_param->type, sizeof(int), 1, fd_output);
			fwrite(&valsize, sizeof valsize, 1, fd_output);

			switch (current_param->type) {
			case TDS_PARAM_INT:
				fwrite(&current_param->ipart, sizeof current_param->ipart, 1, fd_output);
				break;
			case TDS_PARAM_UINT:
				fwrite(&current_param->upart, sizeof current_param->upart, 1, fd_output);
				break;
			case TDS_PARAM_FLOAT:
				fwrite(&current_param->fpart, sizeof current_param->fpart, 1, fd_output);
				break;
			case TDS_PARAM_STRING:
				fwrite(current_param->spart, sizeof current_param->spart / sizeof current_param->spart[0], 1, fd_output);
				break;
			}

			current_param = current_param->next;
		}
	}

	tds_free(str_filename);
	fclose(fd_output);
}

void tds_engine_destroy_objects(struct tds_engine* ptr, const char* type_name) {
	for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
		struct tds_object* cur = ptr->object_buffer->buffer[i].data;

		if (!cur) {
			continue;
		}

		if (!strcmp(cur->type_name, type_name)) {
			tds_object_free(cur);
		}
	}
}

void tds_engine_broadcast(struct tds_engine* ptr, int msg, void* param) {
	for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
		struct tds_object* cur = ptr->object_buffer->buffer[i].data;

		if (!cur) {
			continue;
		}

		tds_object_msg(cur, NULL, msg, param);
	}
}
