#include "script.h"
#include "memory.h"
#include "log.h"

#include <lauxlib.h>
#include <string.h>

struct tds_script* tds_script_create(const char* filename) {
	struct tds_script* output = tds_malloc(sizeof *output);

	output->ctx = luaL_newstate();
	luaL_openlibs(output->ctx);

	if (luaL_loadfile(output->ctx, filename)) {
		tds_logf(TDS_LOG_WARNING, "Failed to load script file [%s]: \n%s\n", filename, lua_tostring(output->ctx, -1));
		lua_pop(output->ctx, 1);
		return output;
	}

	if (lua_pcall(output->ctx, 0, 0, 0)) {
		tds_logf(TDS_LOG_WARNING, "Script execution error: \n%s\n", lua_tostring(output->ctx, -1));
		lua_pop(output->ctx, 1);
	}

	return output;
}

void tds_script_free(struct tds_script* ptr) {
	struct tds_script_string* cur = ptr->strings, *tmp = NULL;

	while (cur) {
		tmp = cur->next;
		tds_free((char*) cur->str);
		tds_free(cur);
		cur = tmp;
	}

	lua_close(ptr->ctx);
	tds_free(ptr);
}

void tds_script_execute_function(struct tds_script* ptr, const char* funcname) {
	lua_getglobal(ptr->ctx, funcname);
	lua_pcall(ptr->ctx, 0, 0, 0);
}

int tds_script_get_var_int(struct tds_script* ptr, const char* varname, int def) {
	lua_getglobal(ptr->ctx, varname);

	if (lua_isnil(ptr->ctx, -1)) {
		return def;
	} else {
		int output = lua_tointeger(ptr->ctx, -1);
		lua_pop(ptr->ctx, 1);
		return output;
	}
}

int tds_script_get_var_bool(struct tds_script* ptr, const char* varname, int def) {
	lua_getglobal(ptr->ctx, varname);

	if (lua_isnil(ptr->ctx, -1)) {
		return def;
	} else {
		int output = lua_toboolean(ptr->ctx, -1);
		lua_pop(ptr->ctx, 1);
		return output;
	}
}

const char* tds_script_get_var_string(struct tds_script* ptr, const char* varname, const char* def) {
	lua_getglobal(ptr->ctx, varname);

	if (lua_isnil(ptr->ctx, -1)) {
		return def;
	} else {
		struct tds_script_string* str = tds_malloc(sizeof *str);

		size_t len = 0;
		const char* str_ptr = lua_tolstring(ptr->ctx, -1, &len);
		char* output = tds_malloc(len + 1);
		memcpy(output, str_ptr, len);
		output[len] = 0;

		str->str = output;
		str->next = ptr->strings;
		ptr->strings = str;

		lua_pop(ptr->ctx, 1);
		return output;
	}
}

void tds_script_var_bool(struct tds_script* ptr, const char* varname, int value) {
	lua_pushboolean(ptr->ctx, value);
	lua_setglobal(ptr->ctx, varname);
}

void tds_script_var_int(struct tds_script* ptr, const char* varname, int value) {
	lua_pushinteger(ptr->ctx, value);
	lua_setglobal(ptr->ctx, varname);
}

void tds_script_var_string(struct tds_script* ptr, const char* varname, char* str, int str_len) {
	lua_pushlstring(ptr->ctx, str, str_len);
	lua_setglobal(ptr->ctx, varname);
}
