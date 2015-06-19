#pragma once

#include <lua.h>

typedef struct lua_State tds_script;

struct tds_script* tds_script_create(const char* filename);
void tds_script_free(struct tds_script* ptr);

void tds_script_bind(struct tds_script* ptr, int (*func)(struct tds_script*), char* name);
void tds_script_call(struct tds_script* ptr, char* name, int args);

void tds_script_pushint(struct tds_script* ptr, int val);
void tds_script_pushfloat(struct tds_script* ptr, float val);
void tds_script_pushstring(struct tds_script* ptr, char* string);

int tds_script_toint(struct tds_script* ptr, int pos);
int tds_script_tofloat(struct tds_script* ptr, int pos);
char* tds_script_tostring(struct tds_script* ptr, int pos); /* Returns internal copy, will very shortly be destroyed. Copy it! */

void tds_script_remove(struct tds_script* ptr, int pos); /* Removes a value from the stack. */
