#pragma once

#include "linmath.h"

#define TDS_TEXT_BATCH_FLAG_CENTERED 1

struct tds_text_batch {
	char* str;
	int str_len;
	struct tds_sprite* font;
	int layer;
	mat4x4 transform;
	float r, g, b, a;
	float x, y, z, angle;
	unsigned int flags;
};

struct tds_text_batch_entry {
	struct tds_text_batch_entry* prev, *next;
	struct tds_text_batch data;
};

struct tds_text {
	struct tds_text_batch_entry* head, *tail;
	int size;
};

struct tds_text* tds_text_create(void);
void tds_text_free(struct tds_text* ptr);

void tds_text_submit(struct tds_text* ptr, struct tds_text_batch* batch);
void tds_text_clear(struct tds_text* ptr);

vec4* tds_text_batch_get_transform(struct tds_text_batch* ptr, int index);
