#pragma once
#include <stdint.h>

#define TDS_SAVESTATE_FILE_PREFIX "save"
#define TDS_SAVESTATE_MAX_DIGITS 2

struct tds_savestate_entry {
	void* data;
	uint32_t data_len, index;
	struct tds_savestate_entry* next;
};

struct tds_savestate {
	unsigned int index;
	struct tds_savestate_entry* head, *tail;
};

struct tds_savestate* tds_savestate_create(void);
void tds_savestate_free(struct tds_savestate* ptr);

void tds_savestate_set_index(struct tds_savestate* ptr, unsigned int index);

struct tds_savestate_entry tds_savestate_get(struct tds_savestate* ptr, unsigned int index);
void tds_savestate_set(struct tds_savestate* ptr, unsigned int index, void* data, unsigned int data_size);

void tds_savestate_write(struct tds_savestate* ptr);
void tds_savestate_read(struct tds_savestate* ptr);
