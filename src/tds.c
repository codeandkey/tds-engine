#include "tds.h"
#include "game/game_input.h"

int main(int argc, char** argv) {
	struct tds_engine* engine_handle = NULL;
	struct tds_engine_desc engine_desc = {0};

	engine_desc.config_filename = "tds.cfg";
	engine_desc.map_filename = "none";
	engine_desc.game_input = tds_get_game_input();
	engine_desc.game_input_size = tds_get_game_input_size();

	engine_handle = tds_engine_create(engine_desc);

	tds_engine_run(engine_handle);
	tds_engine_free(engine_handle);

	tds_memcheck();
	return 0;
}
