#pragma once

#include "../key_map.h"

struct tds_key_map_template* tds_get_game_input(void);
int tds_get_game_input_size(void);

/* Declare all game inputs here, define them in the C source. */

int TDS_GAME_INPUT_QUIT;
int TDS_GAME_INPUT_MOVE_LEFT, TDS_GAME_INPUT_MOVE_RIGHT, TDS_GAME_INPUT_MOVE_UP, TDS_GAME_INPUT_MOVE_DOWN;
