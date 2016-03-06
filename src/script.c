#include "script.h"
#include "memory.h"
#include "log.h"

#include <lauxlib.h>

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
