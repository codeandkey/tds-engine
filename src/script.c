#include "script.h"
#include "memory.h"
#include "log.h"

#include <lauxlib.h>
#include <lualib.h>

static int _tds_script_runtime_error(lua_State* state);

struct tds_script* tds_script_create(const char* filename) {
	struct tds_script* output = luaL_newstate();

	luaL_openlibs(output);
	lua_atpanic(output, _tds_script_runtime_error);

	if (luaL_dofile(output, filename)) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to run script %s : %s.\n", filename, lua_tostring(output, -1));
		return NULL;
	}

	lua_getglobal(output, "testfunc");
	lua_pushliteral(output, "Test World");
	lua_call(output, 1, 1);

	tds_logf(TDS_LOG_MESSAGE, "Lua script : %s\n", lua_tostring(output, -1));

	return output;
}

void tds_script_free(struct tds_script* ptr) {
	lua_close(ptr);
}

int _tds_script_runtime_error(lua_State* state) {
	tds_logf(TDS_LOG_CRITICAL, "Lua panic callback!\n");
	return 0;
}
