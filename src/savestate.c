#include "savestate.h"
#include "log.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void _tds_savestate_flush(struct tds_savestate* ptr);

struct tds_savestate* tds_savestate_create(void) {
	struct tds_savestate* output = tds_malloc(sizeof(struct tds_savestate));
	tds_savestate_read(output);
	return output;
}

void tds_savestate_free(struct tds_savestate* ptr) {
	_tds_savestate_flush(ptr);
	tds_free(ptr);
}

void tds_savestate_set_index(struct tds_savestate* ptr, unsigned int index) {
	if (index != ptr->index) {
		ptr->index = index;
		tds_savestate_read(ptr);
	}
}

struct tds_savestate_entry tds_savestate_get(struct tds_savestate* ptr, unsigned int index) {
	struct tds_savestate_entry* cur = ptr->head, def = {NULL, 0, 0, NULL};

	while (cur) {
		if (cur->index == index) {
			return *cur;
		}

		cur = cur->next;
	}

	return def;
}

void tds_savestate_set(struct tds_savestate* ptr, unsigned int index, void* data, unsigned int data_len) {
	struct tds_savestate_entry* cur = ptr->head;

	while (cur) {
		if (cur->index == index) {
			tds_free(cur->data);
			cur->data = tds_malloc(data_len);
			memcpy(cur->data, data, data_len);
			cur->data_len = data_len;
			return;
		}

		cur = cur->next;
	}

	tds_logf(TDS_LOG_DEBUG, "Savestate index %d wasn't in the list, I will append it.\n", index);

	cur = tds_malloc(sizeof *cur);
	cur->data = tds_malloc(data_len);
	memcpy(cur->data, data, data_len);
	cur->data_len = data_len;
	cur->index = index;

	if (ptr->tail) {
		ptr->tail->next = cur;
	} else {
		ptr->head = cur;
	}

	ptr->tail = cur;
}

void tds_savestate_write(struct tds_savestate* ptr) {
	unsigned int buflen = strlen(TDS_SAVESTATE_FILE_PREFIX) + TDS_SAVESTATE_MAX_DIGITS + 1;
	char* filename = tds_malloc(buflen);

	snprintf(filename, buflen, "%s%d", TDS_SAVESTATE_FILE_PREFIX, ptr->index);

	FILE* fd = fopen(filename, "w");

	if (!fd) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to open file [%s] for writing.\n", filename);
		return;
	}

	struct tds_savestate_entry* cur = ptr->head;

	while (cur) {
		fwrite(&cur->index, sizeof(cur->index), 1, fd);
		fwrite(&cur->data_len, sizeof(cur->data_len), 1, fd);
		fwrite(cur->data, 1, cur->data_len, fd);

		cur = cur->next;
	}

	fclose(fd);
	tds_logf(TDS_LOG_MESSAGE, "Wrote savestate to [%s]\n", filename);
	tds_free(filename);
}

void tds_savestate_read(struct tds_savestate* ptr) {
	unsigned int buflen = strlen(TDS_SAVESTATE_FILE_PREFIX) + TDS_SAVESTATE_MAX_DIGITS + 1;
	char* filename = tds_malloc(buflen);

	snprintf(filename, buflen, "%s%d", TDS_SAVESTATE_FILE_PREFIX, ptr->index);

	FILE* fd = fopen(filename, "r");

	if (!fd) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to open file [%s] for reading.\n", filename);
		return;
	}

	while (!feof(fd)) {
		struct tds_savestate_entry* new_entry = tds_malloc(sizeof(*new_entry));

		if (!fread(&new_entry->index, sizeof(new_entry->index), 1, fd)) {
			tds_free(new_entry);
			break;
		}

		if (!fread(&new_entry->data_len, sizeof(new_entry->data_len), 1, fd)) {
			tds_free(new_entry);
			break;
		}

		tds_logf(TDS_LOG_DEBUG, "Parsed savestate entry %d, size %d\n", new_entry->index, new_entry->data_len);

		new_entry->data = tds_malloc(new_entry->data_len);

		if (!fread(new_entry->data, 1, new_entry->data_len, fd)) {
			tds_free(new_entry->data);
			tds_free(new_entry);
			break;
		}

		if (ptr->tail) {
			ptr->tail->next = new_entry;
		} else {
			ptr->head = new_entry;
		}

		ptr->tail = new_entry;
	}

	fclose(fd);
	tds_free(filename);
}

void _tds_savestate_flush(struct tds_savestate* ptr) {
	struct tds_savestate_entry* tmp;

	while (ptr->head) {
		tds_free(ptr->head->data);
		tmp = ptr->head->next;
		tds_free(ptr->head);
		ptr->head = tmp;
	}

	ptr->tail = NULL;
}
