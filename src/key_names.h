#pragma once

#include <GLFW/glfw3.h>

struct tds_key_name {
	const char* name;
	int id;
};

struct tds_key_name* tds_key_name_get_list(void);
int tds_key_name_get_list_size(void);
int tds_key_name_get_key(char* name);
