#pragma once

/* A sound object contains an AL source and an AL buffer. */
/* Buffers are shared, they should be retrieved from the sound_buffer_cache. */

#include "sound_buffer.h"

struct tds_sound_source {
	unsigned int source;
};

struct tds_sound_source* tds_sound_source_create(void);
void tds_sound_source_free(struct tds_sound_source* ptr);

void tds_sound_source_set_pos(struct tds_sound_source* ptr, float x, float y);
void tds_sound_source_set_vel(struct tds_sound_source* ptr, float x, float y);
void tds_sound_source_set_vol(struct tds_sound_source* ptr, float vol);
void tds_sound_source_set_loop(struct tds_sound_source* ptr, int loop);
void tds_sound_source_load_buffer(struct tds_sound_source* ptr, struct tds_sound_buffer* buf);
void tds_sound_source_play(struct tds_sound_source* ptr);
void tds_sound_source_stop(struct tds_sound_source* ptr);
