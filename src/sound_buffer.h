#pragma once

struct tds_sound_buffer {
	unsigned int buffer_id;
};

struct tds_sound_buffer* tds_sound_buffer_create(char* filename);
void tds_sound_buffer_free(struct tds_sound_buffer* ptr);
