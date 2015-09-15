#include "tds.h"
#include "game/game_input.h"

static void _tds_load_sprites(struct tds_sprite_cache* sc_handle);
static void _tds_load_sounds(struct tds_sound_cache* sndc_handle);
static void _tds_load_object_types(struct tds_object_type_cache* otc_handle);

int main(int argc, char** argv) {
	struct tds_engine* engine_handle = NULL;
	struct tds_engine_desc engine_desc = {0};

	engine_desc.config_filename = "tds.cfg";
	engine_desc.map_filename = "default";
	engine_desc.game_input = tds_get_game_input();
	engine_desc.game_input_size = tds_get_game_input_size();

	engine_desc.func_load_sprites = _tds_load_sprites;
	engine_desc.func_load_sounds = _tds_load_sounds;
	engine_desc.func_load_object_types = _tds_load_object_types;

	engine_handle = tds_engine_create(engine_desc);

	tds_engine_run(engine_handle);
	tds_engine_free(engine_handle);

	tds_memcheck();
	return 0;
}

void _tds_load_sprites(struct tds_sprite_cache* sc_handle) {
}

void _tds_load_sounds(struct tds_sound_cache* sndc_handle) {
}

void _tds_load_object_types(struct tds_object_type_cache* otc_handle) {
}
