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
#include "yxml.h"

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

	output->overlay_handle = tds_overlay_create(output->display_handle->desc.width, output->display_handle->desc.height);
	tds_logf(TDS_LOG_MESSAGE, "Initialized overlay system.\n");

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
		if (strcmp(desc.map_filename, "none")) {
			tds_logf(TDS_LOG_MESSAGE, "Loading initial map [%s].\n", desc.map_filename);
			tds_engine_load(output, desc.map_filename);
		} else {
			tds_logf(TDS_LOG_WARNING, "Not loading a map.\n");
		}
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
	tds_overlay_free(ptr->overlay_handle);
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
		tds_overlay_set_color(ptr->overlay_handle, 1.0f, 0.0f, 0.0f, 0.5f);

		tds_render_clear(ptr->render_handle); /* We clear before executing the draw functions, otherwise the text buffer would be destroyed */
		tds_overlay_clear(ptr->overlay_handle);

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

		tds_render_draw(ptr->render_handle, ptr->world_handle, ptr->overlay_handle);
		tds_display_swap(ptr->display_handle);

		tds_render_clear_lights(ptr->render_handle);
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

	FILE* fd = fopen(str_filename, "r");

	if (!fd) {
		tds_logf(TDS_LOG_WARNING, "Failed to load map %s.\n", mapname);
		tds_free(str_filename);
		return;
	}

	yxml_t* ctx = tds_malloc(sizeof(yxml_t) + TDS_LOAD_BUFFER_SIZE); // We hide the buffer with the YXML context
	yxml_init(ctx, ctx + 1, TDS_LOAD_BUFFER_SIZE);

	int in_layer = 0, in_object = 0, in_parameter = 0, in_data = 0;

	struct tds_object* cur_object = NULL;
	struct tds_object_param* cur_object_param = NULL;

	char obj_type_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};
	char obj_x_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};
	char obj_y_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};
	char obj_width_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};
	char obj_height_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};
	char obj_angle_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};
	char obj_visible_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};

	char* target_attr = obj_type_buf;

	char data_encoding_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};

	char world_width_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};
	char world_height_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};

	int world_width = 1, world_height = 1;

	char world_buffer[TDS_LOAD_WORLD_SIZE + 1] = {0};
	uint8_t* id_buffer = NULL;

	char prop_name_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};
	char prop_val_buf[TDS_LOAD_ATTR_SIZE + 1] = {0};

	int dont_load_world = 0;

	char c_char = 0;
	while ((c_char = fgetc(fd)) != EOF) {

		if (!c_char) {
			break;
		}

		yxml_ret_t r = yxml_parse(ctx, c_char);

		if (r < 0) {
			tds_logf(TDS_LOG_WARNING, "yxml parsing error while loading %s.\n", str_filename);
			tds_free(str_filename);
			tds_free(ctx);
			return;
		}

		switch (r) {
		case YXML_ELEMSTART:
			tds_logf(TDS_LOG_DEBUG, "Starting to parse element %s\n", ctx->elem);
			if (!strcmp(ctx->elem, "object")) {
				in_object = 1;
			}
			if (!strcmp(ctx->elem, "layer")) {
				in_layer = 1;
			}
			if (!strcmp(ctx->elem, "data")) {
				in_data = 1;
			}
			if (!strcmp(ctx->elem, "property")) {
				in_parameter = 1;
			}
			break;
		case YXML_ATTRSTART:
			tds_logf(TDS_LOG_DEBUG, "Starting to parse attribute %s\n", ctx->attr);
			target_attr = NULL;

			if (in_object) {
				if (!strcmp(ctx->attr, "type")) {
					target_attr = obj_type_buf;
				}

				if (!strcmp(ctx->attr, "x")) {
					target_attr = obj_x_buf;
				}

				if (!strcmp(ctx->attr, "y")) {
					target_attr = obj_y_buf;
				}

				if (!strcmp(ctx->attr, "width")) {
					target_attr = obj_width_buf;
				}

				if (!strcmp(ctx->attr, "height")) {
					target_attr = obj_height_buf;
				}

				if (!strcmp(ctx->attr, "visible")) {
					target_attr = obj_visible_buf;
				}

				if (!strcmp(ctx->attr, "angle")) {
					target_attr = obj_angle_buf;
				}
			}

			if (in_layer && !in_data) {
				if (!strcmp(ctx->attr, "width")) {
					target_attr = world_width_buf;
				}

				if (!strcmp(ctx->attr, "height")) {
					target_attr = world_height_buf;
				}
			}

			if (in_data) {
				if (!strcmp(ctx->attr, "encoding")) {
					target_attr = data_encoding_buf;
				}
			}

			if (in_parameter) {
				if (!strcmp(ctx->attr, "name"))	{
					target_attr = prop_name_buf;
				}

				if (!strcmp(ctx->attr, "value")) {
					target_attr = prop_val_buf;
				}
			}
			break;
		case YXML_ATTRVAL:
			if (!target_attr) {
				break;
			}

			if (strlen(target_attr) >= TDS_LOAD_ATTR_SIZE) {
				tds_logf(TDS_LOG_WARNING, "Attribute value too large, truncating! %s=%s..\n", ctx->attr, target_attr);
				break;
			}

			target_attr[strlen(target_attr)] = *(ctx->data);
			break;
		case YXML_ATTREND:
			if (in_data) {
				if (strcmp(data_encoding_buf, "csv")) {
					tds_logf(TDS_LOG_WARNING, "World data should be encoded as CSV. [%s]\n", data_encoding_buf);
					dont_load_world = 1;
				}
			}
			break;
		case YXML_CONTENT:
			if (in_data) {
				if (strlen(world_buffer) >= TDS_LOAD_WORLD_SIZE) {
					tds_logf(TDS_LOG_WARNING, "World data too large!\n");
					break;
				}

				world_buffer[strlen(world_buffer)] = *(ctx->data);
			}
			break;
		case YXML_ELEMEND:
			if (in_object && !in_parameter) {
				in_object = 0;

				struct tds_object_type* type_ptr = tds_object_type_cache_get(ptr->otc_handle, obj_type_buf);

				if (!type_ptr) {
					tds_logf(TDS_LOG_WARNING, "Unknown typename in map file [%s]!\n", type_ptr);
					break;
				}

				tds_logf(TDS_LOG_DEBUG, "Constructing object of type [%s]\n", obj_type_buf);

				float map_x = strtof(obj_x_buf, NULL), map_y = strtof(obj_y_buf, NULL);
				float map_block_size = TDS_WORLD_BLOCK_SIZE * 32.0f;
				float map_width = map_block_size * world_width;
				float map_height = map_block_size * world_height;
				float game_width = TDS_WORLD_BLOCK_SIZE * world_width;
				float game_height = TDS_WORLD_BLOCK_SIZE * world_height;
				float real_width = (strtof(obj_width_buf, NULL) / map_width) * game_width;
				float real_height = (strtof(obj_height_buf, NULL) / map_height) * game_height;
				float real_x = -game_width / 2.0f + (game_width * (map_x / map_width)) + (real_width / 2.0f);
				float real_y = -game_height / 2.0f + (game_height * ((map_height - map_y) / map_height)) - (real_height / 2.0f);

				cur_object = tds_object_create(type_ptr, ptr->object_buffer, ptr->sc_handle, real_x, real_y, 0.0f, cur_object_param);

				cur_object->cbox_width = real_width;
				cur_object->cbox_height = real_height;

				cur_object->visible = strcmp(obj_visible_buf, "0") ? 1 : 0;
				cur_object->angle = strtof(obj_angle_buf, NULL) * 3.141f / 180.0f;

				memset(obj_type_buf, 0, sizeof obj_type_buf / sizeof *obj_type_buf);
				memset(obj_visible_buf, 0, sizeof obj_visible_buf / sizeof *obj_visible_buf);
				memset(obj_angle_buf, 0, sizeof obj_angle_buf / sizeof *obj_angle_buf);
				memset(obj_x_buf, 0, sizeof obj_x_buf / sizeof *obj_x_buf);
				memset(obj_y_buf, 0, sizeof obj_y_buf / sizeof *obj_y_buf);
				memset(obj_width_buf, 0, sizeof obj_width_buf / sizeof *obj_width_buf);
				memset(obj_height_buf, 0, sizeof obj_height_buf / sizeof *obj_height_buf);

				cur_object_param = NULL;
			} else if (in_parameter) {
				in_parameter = 0;

				struct tds_object_param* next_param = tds_malloc(sizeof *next_param);

				next_param->next = cur_object_param;
				cur_object_param = next_param;

				switch (prop_name_buf[0]) {
				default:
					tds_logf(TDS_LOG_WARNING, "Invalid type prefix [%c] in object parameter; default to int\n", prop_name_buf[0]);
				case 'i':
					next_param->type = TDS_PARAM_INT;
					next_param->ipart = strtol(prop_val_buf, NULL, 10);
					break;
				case 'u':
					next_param->type = TDS_PARAM_UINT;
					next_param->upart = strtol(prop_val_buf, NULL, 10);
					break;
				case 'f':
					next_param->type = TDS_PARAM_FLOAT;
					next_param->fpart = strtof(prop_val_buf, NULL);
					break;
				case 's':
					{
						next_param->type = TDS_PARAM_STRING;
						int srclen = strlen(prop_val_buf), writelen = srclen;
						if (srclen > TDS_PARAM_VALSIZE) {
							tds_logf(TDS_LOG_WARNING, "Parameter string longer than %d. Truncating..\n", TDS_PARAM_VALSIZE);
							writelen = TDS_PARAM_VALSIZE;
						}
						memcpy(next_param->spart, prop_val_buf, writelen);
					}
					break;
				}

				next_param->key = strtol(prop_name_buf + 1, NULL, 10);
				tds_logf(TDS_LOG_DEBUG, "Created object parameter with type %c, key %d, valbuf [%s], namebuf [%s]\n", prop_name_buf[0], next_param->key, prop_val_buf, prop_name_buf);

				memset(prop_name_buf, 0, sizeof prop_name_buf / sizeof *prop_name_buf);
				memset(prop_val_buf, 0, sizeof prop_val_buf / sizeof *prop_val_buf);
			} else if (in_layer && in_data) {
				in_data = 0;
			} else if (in_layer && !in_data) {
				if (id_buffer) {
					in_layer = 0;
					tds_logf(TDS_LOG_WARNING, "There was more than one layer in the map file. Only using the first..\n");
					break;
				}

				if (dont_load_world) {
					break;
				}

				in_layer = 0;
				/* Construct the world! */

				world_width = strtol(world_width_buf, NULL, 10);
				world_height = strtol(world_height_buf, NULL, 10);

				id_buffer = tds_malloc(world_width * world_height * sizeof *id_buffer);

				char* c_block = strtok(world_buffer, ",");
				int x = 0, y = world_height - 1;

				do {
					if (y < 0) {
						tds_logf(TDS_LOG_WARNING, "World was larger than map said it was. Ignoring..\n");
						break;
					}

					id_buffer[x++ + y * world_width] = strtol(c_block, NULL, 10);

					if (x >= world_width) {
						--y;
						x = 0;
					}
				} while ((c_block = strtok(NULL, ",")));

				/* The block IDs are stored now in id_buffer, with the correct winding order. */

				tds_world_init(ptr->world_handle, world_width, world_height);
				tds_world_load(ptr->world_handle, id_buffer, world_width, world_height);
			}
			break;
		default:
			break;
		}
	}

	if (id_buffer) {
		tds_free(id_buffer);
	}

	yxml_ret_t ret = yxml_eof(ctx);

	if (ret < 0) {
		tds_logf(TDS_LOG_WARNING, "yxml reported incorrectly formatted map file at EOF!\n");
	}

	tds_free(ctx);
	tds_free(str_filename);
	tds_engine_broadcast(ptr, TDS_MSG_MAP_READY, 0);
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
