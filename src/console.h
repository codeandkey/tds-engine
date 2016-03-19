#pragma once
#include "sprite.h"

/* This was originally going to be a part of the ECS, but adding global objects would complicate the architecture more than just hard-coding this in. */
/* Also, having a global console to work with is seriously useful. */

#define TDS_CONSOLE_ROWS 4
#define TDS_CONSOLE_COLS 32
#define TDS_CONSOLE_INPUT_SIZE 128

struct tds_console {
	int rows, cols, curs_row, curs_col;
	char** buffers;
	int enabled;
	char input_buf[TDS_CONSOLE_INPUT_SIZE];
	int input_ind;
};

struct tds_console* tds_console_create(void);
void tds_console_free(struct tds_console* ptr);

int tds_console_char_pressed(struct tds_console* ptr, unsigned int chr); // Returns whether the console actually used the input.
void tds_console_key_pressed(struct tds_console* ptr, int key);
void tds_console_draw(struct tds_console* ptr);
void tds_console_print(struct tds_console* ptr, const char* str);
