#pragma once

struct tds_lua_context {
};

struct tds_lua_context* tds_lua_context_create(const char* filename);
void tds_lua_context_free(struct tds_lua_context* ptr);

int tds_lua_context_getint(struct tds_lua_context* ptr, char* varname);
float tds_lua_context_getfloat(struct tds_lua_context* ptr, char* varname);
void tds_lua_context_setint(struct tds_lua_context* ptr, char* varname, int val);
void tds_lua_context_setfloat(struct tds_lua_context* ptr, char* varname, float val);
void tds_lua_context_call(struct tds_lua_context* ptr, char* funcname);
void tds_lua_context_bind(struct tds_lua_context* ptr, void(*func)(void), char* varname);
