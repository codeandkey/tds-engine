#pragma once

#include <GLFW/glfw3.h>

struct tds_display_desc {
	int width, height, fs, vsync;
};

struct tds_display {
	GLFWwindow* win_handle;
	struct tds_display_desc desc;
};

struct tds_display* tds_display_create(struct tds_display_desc desc);
void tds_display_free(struct tds_display* ptr);

void tds_display_swap(struct tds_display* ptr);
void tds_display_update(struct tds_display* ptr);
int tds_display_get_close(struct tds_display* ptr);
