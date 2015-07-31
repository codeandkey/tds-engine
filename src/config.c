#include "config.h"
#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct tds_config* tds_config_create(const char* filename) {
	struct tds_config* output = tds_malloc(sizeof(struct tds_config));
	FILE* fp = fopen(filename, "r");

	if (!fp) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to open config file %s for reading.\n", filename);
		return NULL;
	}

	char t_line_buffer[TDS_CONFIG_KEY_SIZE + TDS_CONFIG_VALUE_SIZE + 2048];
	char* t_key, *t_value;

	output->pair_count = 0;

	while (fgets(t_line_buffer, sizeof(t_line_buffer), fp)) {
		if (*t_line_buffer == '#') {
			continue;
		}

		if (strlen(t_line_buffer) && t_line_buffer[strlen(t_line_buffer) - 1] == '\n') {
			t_line_buffer[strlen(t_line_buffer) - 1] = 0;
		}

		if (!strlen(t_line_buffer)) {
			continue;
		}

		t_key = strtok(t_line_buffer, "=");
		t_value = strtok(NULL, "=");

		strncpy(output->key_buffer[output->pair_count], t_key, TDS_CONFIG_KEY_SIZE);
		strncpy(output->value_buffer[output->pair_count++], t_value, TDS_CONFIG_VALUE_SIZE);
	}

	fclose(fp);
	return output;
}

struct tds_config* tds_config_create_cmd(int argc, char** argv) {
	struct tds_config* output = tds_malloc(sizeof(struct tds_config));
	output->pair_count = 0;

	argv += 1; /* Disregard the first argument. */
	argc -= 1;

	int read_key = 1;
	char* key, *value;

	for (int i = 0; i < argc; ++i) {
		if (read_key) {
			key = argv[i];
		} else {
			value = argv[i];

			strncpy(output->key_buffer[output->pair_count], key, TDS_CONFIG_KEY_SIZE);
			strncpy(output->value_buffer[output->pair_count++], value, TDS_CONFIG_VALUE_SIZE);
		}

		read_key = !read_key;
	}

	return output;
}

void tds_config_free(struct tds_config* ptr) {
	tds_free(ptr);
}

char* tds_config_get_string(struct tds_config* ptr, const char* key) {
	for (int i = 0; i < ptr->pair_count; i++) {
		if (!strcmp(ptr->key_buffer[i], key)) {
			return ptr->value_buffer[i];
		}
	}

	tds_logf(TDS_LOG_CRITICAL, "Key \"%s\" not found in buffers.\n", key);
	return 0;
}

int tds_config_get_int(struct tds_config* ptr, const char* key) {
	char* value = tds_config_get_string(ptr, key);

	if (!strcmp(value, "no") || !strcmp(value, "false")) {
		return 0;
	}

	if (!strcmp(value, "yes") || !strcmp(value, "true")) {
		return 1;
	}

	return (int) strtol(value, NULL, 10);
}

float tds_config_get_float(struct tds_config* ptr, const char* key) {
	char* value = tds_config_get_string(ptr, key);

	return (float) strtof(value, NULL);
}
