#pragma once

/* The TDS key map abstracts key/axis codes from game input. */

/*
 * The input problem:
 *
 * We must be able to dynamically modify the input mapping. (therefore, it must not be a constant buffer and must have name IDs for inputs)
 * We must have constant-time access to the input mapping from the game (with a predefined set of IDs), therefore the input mapping must be created before modification
 *
 * Solution:
 *
 * Have a game-specific mapping generated at the beginning of the game, provided by an array.
 * After the game-specific mapping is generated, allow configs (or global mods) to modify it.
 *
 * The game will have constant access, the config mapper will have linear access, and we will even be able to warn users when their configs are wrong.
 * The key mapper has no knowledge of what a key code will be used for.. at all. That is up to the object making the queries.
 * This is simply a configurable mapping of game inputs to integer codes. Nothing more, nothing less.
 */

struct tds_key_map_entry {
	const char* name;
	int key;
};

struct tds_key_map_template {
	const char* name;
	const char* keyname;
};

struct tds_key_map {
	struct tds_key_map_entry* entry_buffer;
	int entry_count;
};

struct tds_key_map* tds_key_map_create(struct tds_key_map_template* def, int def_count);
void tds_key_map_free(struct tds_key_map* ptr);

void tds_key_map_load(struct tds_key_map* ptr, const char* config);
void tds_key_map_reassign(struct tds_key_map* ptr, const char* name, int key);
