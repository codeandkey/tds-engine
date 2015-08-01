#pragma once

#include "display.h"
#include <GLFW/glfw3.h>

struct tds_input {
	struct GLFWwindow* window_handle;
	int kb_state[512], kb_state_last[512];
	int controller_state[32], controller_state_last[32];
	int mb_state[16], mb_state_last[16];
	float controller_axis_state[32];
	int last_char;
	double mx, my, mx_last, my_last;
};

struct tds_input* tds_input_create(struct tds_display* display_handle);
void tds_input_free(struct tds_input* ptr);

void tds_input_update(struct tds_input* ptr);
int tds_input_get_controller(struct tds_input* ptr);
int tds_input_get_char(struct tds_input* ptr);
