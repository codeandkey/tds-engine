#pragma once

/* The engine configuration language is as simple as it gets.
 *
 * Key=Value
 * Key2=Value
 *
 * Strings are not enclosed in quotations. Everything is read up to the newline (or EOF).
 */

#define TDS_CONFIG_PAIRS 16
#define TDS_CONFIG_KEY_SIZE 64
#define TDS_CONFIG_VALUE_SIZE 64

struct tds_config {
	char value_buffer[TDS_CONFIG_PAIRS][TDS_CONFIG_VALUE_SIZE];
	char key_buffer[TDS_CONFIG_PAIRS][TDS_CONFIG_KEY_SIZE];

	int pair_count;
};

struct tds_config* tds_config_create(const char* filename);
void tds_config_free(struct tds_config* ptr);

int tds_config_get_int(struct tds_config* ptr, const char* key);
float tds_config_get_float(struct tds_config* ptr, const char* key);
char* tds_config_get_string(struct tds_config* ptr, const char* key);
