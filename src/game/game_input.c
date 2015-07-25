#include "game_input.h"

int TDS_GAME_INPUT_QUIT = 0;

struct tds_key_map_template _tds_game_input[] = {
	{"quit", "escape"}
};

struct tds_key_map_template* tds_get_game_input(void) {
	return _tds_game_input;
}

int tds_get_game_input_size(void) {
	return sizeof(_tds_game_input) / sizeof(struct tds_key_map_template);
}
