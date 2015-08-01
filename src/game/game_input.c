#include "game_input.h"

int TDS_GAME_INPUT_QUIT = 0;
int TDS_GAME_INPUT_MOVE_LEFT = 1;
int TDS_GAME_INPUT_MOVE_RIGHT = 2;
int TDS_GAME_INPUT_MOVE_UP = 3;
int TDS_GAME_INPUT_MOVE_DOWN = 4;
int TDS_GAME_INPUT_ATTACK = 5;

struct tds_key_map_template _tds_game_input[] = {
	{"quit", "escape"},
	{"left", "A"},
	{"right", "D"},
	{"up", "W"},
	{"down", "S"},
	{"attack", "leftmouse"},
};

struct tds_key_map_template* tds_get_game_input(void) {
	return _tds_game_input;
}

int tds_get_game_input_size(void) {
	return sizeof(_tds_game_input) / sizeof(struct tds_key_map_template);
}
