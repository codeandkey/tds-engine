#include "engine.h"
#include "config.h"
#include "display.h"
#include "memory.h"
#include "log.h"

struct tds_engine* tds_engine_create(struct tds_engine_desc desc) {
	struct tds_engine* output = tds_malloc(sizeof(struct tds_engine));
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

	/* Free configs */
	tds_config_free(conf);

	tds_logf(TDS_LOG_MESSAGE, "Done initializing everything.\n");
	return output;
}

void tds_engine_free(struct tds_engine* ptr) {
	tds_logf(TDS_LOG_MESSAGE, "Freeing engine structure and subsystems.\n");

	tds_display_free(ptr->display_handle);
	tds_texture_cache_free(ptr->tc_handle);
	tds_free(ptr);
}

void tds_engine_run(struct tds_engine* ptr) {
	int running = 1;

	tds_logf(TDS_LOG_MESSAGE, "Starting engine mainloop.\n");

	while (running) {
		running &= !tds_display_get_close(ptr->display_handle);

		tds_display_update(ptr->display_handle);
		tds_display_swap(ptr->display_handle);
	}

	tds_logf(TDS_LOG_MESSAGE, "Finished engine mainloop.\n");
}
