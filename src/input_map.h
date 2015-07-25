#pragma once

#include "input.h"

struct tds_input_map {
	struct tds_input* input_handle;
	int use_controller;
};

struct tds_input_map* tds_input_map_create(struct tds_input* input_handle);
void tds_input_map_free(struct tds_input_map* ptr);

float tds_input_map_get_axis(struct tds_input_map* ptr, int key_low, int key_high, int axis);

int tds_input_map_get_key(struct tds_input_map* ptr, int key, int controller_button);
int tds_input_map_get_key_pressed(struct tds_input_map* ptr, int key, int controller_button);
int tds_input_map_get_key_released(struct tds_input_map* ptr, int key, int controller_button);

char tds_input_map_get_char(struct tds_input_map* ptr); /* Gets the character typed by the user. Returns 0 when there is no input to be read. */
