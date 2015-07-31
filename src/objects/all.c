#include "all.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct tds_object_type* _tds_object_type_list[] = {
	&tds_obj_system_type,
	&tds_obj_player_type,
	&tds_obj_cursor_type,
	&tds_obj_player_legs_type,
	&tds_obj_wall_type,
};

struct tds_object_type** tds_object_type_get_list(void) {
	return _tds_object_type_list;
}

int tds_object_type_get_count(void) {
	return sizeof(_tds_object_type_list) / sizeof(struct tds_object_type*);
}

struct tds_object_type* tds_object_type_get_by_name(const char* name) {
	int count = tds_object_type_get_count();

	for (int i = 0; i < count; ++i) {
		if (!strcmp(name, tds_object_type_get_list()[i]->type_name)) {
			return tds_object_type_get_list()[i];
		}
	}

	return NULL;
}
