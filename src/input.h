#pragma once

#include "display.h"
#include <GLFW/glfw3.h>

struct tds_input {
	struct GLFWwindow* window_handle;
	int kb_state[312];
	int controller_state[32];
	int mb_state[5];
	float controller_axis_state[32];
	double mx, my, mx_last, my_last;
};

struct tds_input* tds_input_create(struct tds_display* display_handle);
void tds_input_free(struct tds_input* ptr);

void tds_input_update(struct tds_input* ptr);
int tds_input_get_controller(struct tds_input* ptr);
int tds_input_get_char(struct tds_input* ptr);
void tds_input_forget_char(struct tds_input* ptr);
void tds_input_set_mouse(struct tds_input* ptr, double x, double y);
