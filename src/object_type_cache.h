#pragma once

#include "object.h"

struct tds_object_type_cache_link {
	struct tds_object_type* data;
	struct tds_object_type_cache_link* next, *prev;

	const char* name;
};

struct tds_object_type_cache {
	struct tds_object_type_cache_link* head, *tail;
};

struct tds_object_type_cache* tds_object_type_cache_create(void);
void tds_object_type_cache_free(struct tds_object_type_cache* ptr);

void tds_object_type_cache_add(struct tds_object_type_cache* ptr, const char* object_type_name, struct tds_object_type* obj);
struct tds_object_type* tds_object_type_cache_get(struct tds_object_type_cache* ptr, const char* object_type_name);
