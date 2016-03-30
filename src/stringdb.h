#pragma once

/*
 * the TDS string database manages string literals in game.
 * this allows for easy string management with objects and language switching.
 * the whole db operates on several linked lists
 */

#define TDS_STRINGDB_PREFIX "res/strings/"
#define TDS_STRINGDB_BUFLEN 512

#define TDS_STRING_FORMAT_SPEC       '^'
#define TDS_STRING_FORMAT_TYPE_COLOR 'c'
#define TDS_STRING_FORMAT_TYPE_SHAKE 's'
#define TDS_STRING_FORMAT_TYPE_END   'e'
#define TDS_STRING_FORMAT_TYPE_WAVE  'w'

/*
 * ^cRRGGBB - red, green, blue
 * ^wSSWWAA - speed, wavelength, amplitude
 * ^sII     - intensity
 * ^e       - end wave or shake
 */

struct tds_string_format {
	int pos, type;
	int fields[4];
	struct tds_string_format* next;
};

struct tds_string_format_fields {
	int type, fields;
};

extern struct tds_string_format_fields tds_string_format_field_counts[];

struct tds_string {
	char* data;
	int len;
	struct tds_string_format* formats;
};

struct tds_string_db_offset_entry_string { /* A single string entry in a db offset. */
	struct tds_string str;
	struct tds_string_db_offset_entry_string* next;
};

struct tds_string_db_offset_entry {
	int offset_id, string_count;
	struct tds_string_db_offset_entry_string* strings;
	struct tds_string_db_offset_entry* next;
};

struct tds_string_db_entry { /* Structure for each CID that appears in the DB file. */
	char* id;
	int id_len;
	int offset_count;
	struct tds_string_db_offset_entry* offsets;
	struct tds_string_db_entry* next;
};

struct tds_stringdb {
	int entry_count;
	struct tds_string_db_entry* entries;

	struct tds_string error_entry, error_offset, error_empty;
};

struct tds_stringdb* tds_stringdb_create(const char* filename);
void tds_stringdb_free(struct tds_stringdb* ptr);

struct tds_string* tds_stringdb_get(struct tds_stringdb* ptr, char* id, int id_len, int index);
