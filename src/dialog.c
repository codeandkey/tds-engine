#include "dialog.h"
#include "engine.h"
#include "render_flat.h"
#include "log.h"
#include "msg.h"
#include "memory.h"
#include "stringdb.h"
#include "texture_cache.h"
#include "texture.h"

struct tds_dialog* tds_dialog_create(const char* seq_filename, struct tds_texture* portrait_frame, struct tds_font* font) {
	struct tds_dialog* output = tds_malloc(sizeof *output);

	output->sequences = NULL;
	output->cur_entry = NULL;
	output->portrait_frame = portrait_frame;

	output->cp = tds_clock_get_point();
	output->cur_entry_pos = 0;
	output->font = font;

	/* Now, we load in the sequences file. */

	char* final_filename = tds_malloc(strlen(seq_filename) + strlen(TDS_DIALOG_PREFIX) + 1);
	memcpy(final_filename, TDS_DIALOG_PREFIX, strlen(TDS_DIALOG_PREFIX));
	memcpy(final_filename + strlen(TDS_DIALOG_PREFIX), seq_filename, strlen(seq_filename));
	final_filename[strlen(seq_filename) + strlen(TDS_DIALOG_PREFIX)] = 0;

	char buf[TDS_DIALOG_FILE_BUFLEN] = {0};
	FILE* fd = fopen(final_filename, "r");

	if (!fd) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to open sequence file [%s] for reading\n", final_filename);
		return NULL;
	}

	tds_free(final_filename);

	struct tds_dialog_sequence* cur_sequence = NULL;

	while (fgets(buf, sizeof buf / sizeof *buf, fd)) {
		buf[strcspn(buf, "\n")] = 0;

		if (buf[0] == '@') {
			/* Sequence name incoming! */
			cur_sequence = tds_malloc(sizeof *cur_sequence);

			int buflen = strlen(buf + 1), cpylen = buflen;
			if (buflen >= TDS_DIALOG_SEQ_NAMELEN) {
				cpylen = TDS_DIALOG_SEQ_NAMELEN - 1;
			}

			memcpy(cur_sequence->name, buf + 1, cpylen);
			cur_sequence->name[TDS_DIALOG_SEQ_NAMELEN - 1] = 0;

			cur_sequence->next = output->sequences;
			cur_sequence->head = cur_sequence->tail = NULL;
			output->sequences = cur_sequence;
		}

		if (buf[0] == ':') {
			if (!cur_sequence) {
				tds_logf(TDS_LOG_WARNING, "No active sequence near [%s], ignoring entry!\n", buf);
				continue;
			}

			struct tds_dialog_seq_entry* new_entry = tds_malloc(sizeof *new_entry);

			char* stringdb_index = strtok(buf + 1, ":"), *stringdb_offset = strtok(NULL, ":"), *texture_name = strtok(NULL, ":"), *saveptr = NULL;

			if (!stringdb_index || !stringdb_offset || !texture_name) {
				tds_logf(TDS_LOG_WARNING, "Invalid format near [%s], ignoring entry\n", buf);
				continue;
			}

			int stringdb_offset_val = strtol(stringdb_offset, &saveptr, 10);
			new_entry->dialog_string = tds_stringdb_get(tds_engine_global->stringdb_handle, stringdb_index, strlen(stringdb_index), stringdb_offset_val);
			new_entry->texture_portrait = tds_texture_cache_get(tds_engine_global->tc_handle, texture_name, -1, -1, 0, 0);
			new_entry->next = NULL;

			if (!cur_sequence->head) {
				cur_sequence->head = new_entry;
			}

			if (cur_sequence->tail) {
				cur_sequence->tail->next = new_entry;
			}

			cur_sequence->tail = new_entry;
		}
	}

	return output;
}

void tds_dialog_free(struct tds_dialog* ptr) {
	struct tds_dialog_sequence* cur_sequence = ptr->sequences, *tmp_sequence = NULL;

	while (cur_sequence) {
		struct tds_dialog_seq_entry* cur_entry = cur_sequence->head, *tmp_entry = NULL;

		while(cur_entry) {
			tmp_entry = cur_entry->next;
			tds_free(cur_entry);
			cur_entry = tmp_entry;
		}

		tmp_sequence = cur_sequence->next;
		tds_free(cur_sequence);
		cur_sequence = tmp_sequence;
	}

	tds_free(ptr);
}

void tds_dialog_start_sequence(struct tds_dialog* ptr, char* name) {
	int name_len = strlen(name);

	if (name_len >= TDS_DIALOG_SEQ_NAMELEN) {
		return;
	}

	struct tds_dialog_sequence* cur_sequence = ptr->sequences;

	while (cur_sequence) {
		if (!strncmp(cur_sequence->name, name, name_len)) {
			/* Name matches, prepare this sequence. */
			ptr->cur_entry = cur_sequence->head;
			ptr->cur_sequence = cur_sequence;
			ptr->cur_entry_pos = 0;
			ptr->cp = tds_clock_get_point();
			tds_engine_broadcast(tds_engine_global, TDS_MSG_DIALOG_START, ptr->cur_sequence->name);
			return;
		}

		cur_sequence = cur_sequence->next;
	}

	tds_logf(TDS_LOG_WARNING, "Did not find sequence name [%s]\n", name);
}

int tds_dialog_get_active(struct tds_dialog* ptr) {
	return (ptr->cur_entry != NULL);
}

void tds_dialog_send_keypress(struct tds_dialog* ptr) {
	if (!ptr->cur_entry) {
		return;
	}

	if (ptr->cur_entry_pos == ptr->cur_entry->dialog_string->len) {
		ptr->cur_entry = ptr->cur_entry->next;

		if (!ptr->cur_entry) {
			tds_engine_broadcast(tds_engine_global, TDS_MSG_DIALOG_STOP, ptr->cur_sequence->name);
		}
	} else {
		ptr->cur_entry_pos = ptr->cur_entry->dialog_string->len;
	}
}

void tds_dialog_render(struct tds_dialog* ptr) {
	if (!ptr->cur_entry) {
		return;
	}

	int next_pos = tds_clock_get_ms(ptr->cp) / TDS_DIALOG_CHAR_INTERVAL;

	if (next_pos > ptr->cur_entry_pos) {
		ptr->cur_entry_pos = next_pos;
	}

	if (ptr->cur_entry_pos > ptr->cur_entry->dialog_string->len) {
		ptr->cur_entry_pos = ptr->cur_entry->dialog_string->len;
	}

	tds_render_flat_set_mode(tds_engine_global->render_flat_overlay_handle, TDS_RENDER_COORD_REL_SCREENSPACE);
	tds_render_flat_set_color(tds_engine_global->render_flat_overlay_handle, 1.0f, 1.0f, 1.0f, 1.0f);
	tds_render_flat_text(tds_engine_global->render_flat_overlay_handle, ptr->font, ptr->cur_entry->dialog_string->data, ptr->cur_entry_pos, -0.9f, 0.9f, TDS_RENDER_LALIGN, ptr->cur_entry->dialog_string->formats);

	tds_logf(TDS_LOG_DEBUG, "rendering dialog text [%.*s]\n", ptr->cur_entry_pos, ptr->cur_entry->dialog_string->data);
}
