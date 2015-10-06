#include "key_map.h"
#include "config.h"
#include "memory.h"
#include "log.h"
#include "key_names.h"

#include <stdlib.h>
#include <string.h>

void _tds_key_map_load_config(struct tds_key_map* ptr, const char* config);

struct tds_key_map* tds_key_map_create(struct tds_key_map_template* def, int def_count) {
	struct tds_key_map* output = tds_malloc(sizeof(struct tds_key_map));

	if (!def) {
		tds_logf(TDS_LOG_WARNING, "No template provided to key map! Buffers will be empty.\n");

		output->entry_buffer = NULL;
		output->entry_count = 0;

		return output;
	}

	output->entry_buffer = tds_malloc(sizeof(struct tds_key_map) * def_count);
	output->entry_count = def_count;

	for (int i = 0; i < def_count; ++i) {
		output->entry_buffer[i].name = def[i].name;
		output->entry_buffer[i].key = tds_key_name_get_key((char*) def[i].keyname);

		tds_logf(TDS_LOG_DEBUG, "Mapping game input %s to key %d [%s]\n", def[i].name, output->entry_buffer[i].key, def[i].keyname);
	}

	return output;
}

void tds_key_map_free(struct tds_key_map* ptr) {
	if (ptr->entry_buffer) {
		tds_free(ptr->entry_buffer);
	}

	tds_free(ptr);
}

void tds_key_map_load(struct tds_key_map* ptr, const char* config) {
	struct tds_config* cfg = tds_config_create(config);

	for (int c = 0; c < cfg->pair_count; ++c) {
		tds_key_map_reassign(ptr, cfg->key_buffer[c], tds_key_name_get_key(cfg->value_buffer[c]));
	}

	tds_config_free(cfg);
}

void tds_key_map_reassign(struct tds_key_map* ptr, const char* name, int id) {
	for (int i = 0; i < ptr->entry_count; ++i) {
		if (!strcmp(name, ptr->entry_buffer[i].name)) {
			ptr->entry_buffer[i].key = id;
		}
	}

	tds_logf(TDS_LOG_WARNING, "No input named [%s] found in mapping");
}

int tds_key_map_get(struct tds_key_map* ptr, int index) {
	return ptr->entry_buffer[index].key;
}
