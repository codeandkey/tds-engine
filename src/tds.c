#include "tds.h"

int main(int argc, char** argv) {
	struct tds_engine* engine_handle = NULL;
	struct tds_engine_desc engine_desc = {0};

	engine_desc.config_filename = "tds.cfg";
	engine_desc.map_filename = "none";

	engine_handle = tds_engine_create(engine_desc);

	/* Test code. */
	struct tds_sprite* test_sprite = tds_sprite_create(tds_texture_cache_get(engine_handle->tc_handle, "test.png", 32, 32), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	tds_sprite_free(test_sprite);
	/* End test code. */

	tds_engine_run(engine_handle);
	tds_engine_free(engine_handle);

	tds_memcheck();
	return 0;
}
