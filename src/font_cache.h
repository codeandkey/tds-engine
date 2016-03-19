#pragma once

#include "font.h"

struct tds_font_cache_link {
	struct tds_font* data;
	struct tds_font_cache_link* next, *prev;

	const char* name;
};

struct tds_font_cache {
	struct tds_font_cache_link* head, *tail;
};

struct tds_font_cache* tds_font_cache_create(void);
void tds_font_cache_free(struct tds_font_cache* ptr);

void tds_font_cache_add(struct tds_font_cache* ptr, const char* font_name, struct tds_font* obj);
struct tds_font* tds_font_cache_get(struct tds_font_cache* ptr, const char* font_name);
