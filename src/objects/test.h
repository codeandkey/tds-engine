#pragma once

#include "../object.h"

void tds_obj_test_init(struct tds_object* ptr);
void tds_obj_test_destroy(struct tds_object* ptr);
void tds_obj_test_draw(struct tds_object* ptr);
void tds_obj_test_update(struct tds_object* ptr);
void tds_obj_test_msg(struct tds_object* ptr, struct tds_object* from, int msg, void* data);

struct tds_obj_test_data {
	int testvar;
};

struct tds_object_type tds_obj_test_type;
