#pragma once

/*
 * TDS uses Lua as a scripting and configuration language.
 */

#include <lua.h>
#include <lualib.h>

struct tds_script_string {
	const char* str;
	struct tds_script_string* next;
};

struct tds_script {
	lua_State* ctx;
	struct tds_script_string* strings;
};

struct tds_script* tds_script_create(const char* filename);
void tds_script_free(struct tds_script* ptr);

void tds_script_execute(struct tds_script* ptr);
void tds_script_execute_function(struct tds_script* ptr, const char* funcname);

int tds_script_get_var_bool(struct tds_script* ptr, const char* varname, int def);
int tds_script_get_var_int(struct tds_script* ptr, const char* varname, int def);
const char* tds_script_get_var_string(struct tds_script* ptr, const char* varname, const char* def);
void tds_script_set_var_bool(struct tds_script* ptr, const char* varname, int value);
void tds_script_set_var_int(struct tds_script* ptr, const char* varname, int value);
void tds_script_set_var_string(struct tds_script* ptr, char* buf, int buflen);
