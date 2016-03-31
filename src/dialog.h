#pragma once

#include "clock.h"
#include "font.h"

/*
 * TDS dialog subsystem
 * this is being implemented experimentally in engine space because it does not fit well into the object model and requires lots of file parsing for sequencing.
 *
 * Sequence file format:
 *
 * @sequence_name
 * :stringdb_index:stringdb_offset:portrait_texture_index
 * ...
 */

#define TDS_DIALOG_PREFIX "res/seq/"

#define TDS_DIALOG_RENDER_TOP_LEFT 0
#define TDS_DIALOG_RENDER_TOP_RIGHT 1
#define TDS_DIALOG_RENDER_BOTTOM_LEFT 2
#define TDS_DIALOG_RENDER_BOTTOM_RIGHT 3

#define TDS_DIALOG_SEQ_NAMELEN 32
#define TDS_DIALOG_FILE_BUFLEN 256

#define TDS_DIALOG_CHAR_INTERVAL 30

struct tds_dialog_seq_entry {
	struct tds_string* dialog_string;
	struct tds_texture* texture_portrait;
	struct tds_dialog_seq_entry* next;
};

struct tds_dialog_sequence {
	char name[TDS_DIALOG_SEQ_NAMELEN];
	struct tds_dialog_sequence* next;
	struct tds_dialog_seq_entry* head, *tail;
};

struct tds_dialog {
	struct tds_dialog_sequence* sequences, *cur_sequence;
	struct tds_dialog_seq_entry* cur_entry; /* Set when a sequence starts. */
	struct tds_texture* portrait_frame;
	struct tds_font* font;
	int cur_entry_pos; /* Current position of the target string. */
	tds_clock_point cp; /* Used for all sorts of things. */
};

struct tds_dialog* tds_dialog_create(const char* seq_filename, struct tds_texture* portrait_frame, struct tds_font* font);
void tds_dialog_free(struct tds_dialog* ptr);

void tds_dialog_start_sequence(struct tds_dialog* ptr, char* name);
int tds_dialog_get_active(struct tds_dialog* ptr);

/* Any dialog updating keypress will have the same action -- skip to the end if the cursor is not at the end and go to the next sequence if it is. */
void tds_dialog_send_keypress(struct tds_dialog* ptr);

void tds_dialog_render(struct tds_dialog* ptr);
