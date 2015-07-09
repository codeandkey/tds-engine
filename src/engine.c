#include "engine.h"
#include "config.h"
#include "display.h"
#include "object.h"
#include "handle.h"
#include "memory.h"
#include "clock.h"
#include "log.h"

#include "objects/all.h"
#include "objects/test.h"

#include <string.h>

#define TDS_ENGINE_TIMESTEP 144.0f

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

	output->state.mapname = "none";
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

	tds_logf(TDS_LOG_MESSAGE, "Loaded display description. Video mode : %d by %d, FS %s, VSYNC %s\n", display_desc.width, display_desc.height, display_desc.fs ? "on" : "off", display_desc.vsync ? "on" : "off");
	output->display_handle = tds_display_create(display_desc);
	tds_logf(TDS_LOG_MESSAGE, "Created display.\n");
	/* End display subsystem */

	output->tc_handle = tds_texture_cache_create();
	tds_logf(TDS_LOG_MESSAGE, "Initialized texture cache.\n");

	output->object_buffer = tds_handle_manager_create(1024);
	tds_logf(TDS_LOG_MESSAGE, "Initialized object buffer.\n");

	/* Free configs */
	tds_config_free(conf);

	tds_logf(TDS_LOG_MESSAGE, "Done initializing everything.\n");
	return output;
}

void tds_engine_free(struct tds_engine* ptr) {
	tds_logf(TDS_LOG_MESSAGE, "Freeing engine structure and subsystems.\n");

	tds_engine_flush_objects(ptr);

	tds_display_free(ptr->display_handle);
	tds_texture_cache_free(ptr->tc_handle);
	tds_handle_manager_free(ptr->object_buffer);
	tds_free(ptr);
}

void tds_engine_run(struct tds_engine* ptr) {
	int running = 1;

	tds_logf(TDS_LOG_MESSAGE, "Starting engine mainloop.\n");

	{
		/* Test code to do stuff. */
		struct tds_object* obj = tds_object_create(&tds_obj_test_type, ptr->object_buffer, NULL, 0.0f, 0.0f, 0.0f, NULL);

		/* Not to be in final game. */
	}

	/* The game, like 'hunter' will use an accumulator-based approach with a fixed timestep.
	 * to support higher-refresh rate monitors, the timestep will be set to 144hz. */

	/* The game logic will update on a time-dependent basis, but the game will draw whenever possible. */

	tds_clock_point dt_point = tds_clock_get_point();
	double accumulator = 0.0f;
	double timestep_ms = 1000.0f / (double) TDS_ENGINE_TIMESTEP;

	while (running) {
		running &= !tds_display_get_close(ptr->display_handle);

		double delta_ms = tds_clock_get_ms(dt_point);
		dt_point = tds_clock_get_point();
		accumulator += delta_ms;

		tds_logf(TDS_LOG_MESSAGE, "frame : accum = %f ms, delta_ms = %f ms, timestep = %f\n", accumulator, delta_ms, timestep_ms);

		tds_display_update(ptr->display_handle);

		while (accumulator >= timestep_ms) {
			accumulator -= timestep_ms;

			/* Run game update logic. */

			for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
				struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[i].data;

				if (!target) {
					continue;
				}

				target->func_update(target);
			}
		}

		/* Run game draw logic. */
		for (int i = 0; i < ptr->object_buffer->max_index; ++i) {
			struct tds_object* target = (struct tds_object*) ptr->object_buffer->buffer[i].data;

			if (!target) {
				continue;
			}

			target->func_draw(target);
		}

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
		if (!strcmp(((struct tds_object*) ptr->object_buffer->buffer[i].data)->type_name, typename)) {
			return ptr->object_buffer->buffer[i].data;
		}
	}

	return NULL;
}
