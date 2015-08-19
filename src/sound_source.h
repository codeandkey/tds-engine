#pragma once

/* A sound object contains an AL source and an AL buffer. */
/* Buffers are shared, they should be retrieved from the sound_buffer_cache. */

struct tds_sound_source {
	unsigned int source;
};

struct tds_sound_source* tds_sound_source_create(void);
void tds_sound_source_free(struct tds_sound_source* ptr);

void tds_sound_source_set_pos(float x, float y, float z);
void tds_sound_source_set_vel(float x, float y, float z);
