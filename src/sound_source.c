#include "sound_source.h"
#include "memory.h"
#include "log.h"

#include <AL/al.h>

struct tds_sound_source* tds_sound_source_create(void) {
	struct tds_sound_source* output = tds_malloc(sizeof(struct tds_sound_source));

	alGenSources(1, &output->source);

	alSourcef(output->source, AL_PITCH, 1.0f);
	alSourcef(output->source, AL_GAIN, 1.0f);
	alSource3f(output->source, AL_POSITION, 0.0f, 0.0f, 0.0f);
	alSource3f(output->source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSourcei(output->source, AL_LOOPING, 0);

	return output;
}

void tds_sound_source_free(struct tds_sound_source* ptr) {
	alDeleteSources(1, &ptr->source);
	tds_free(ptr);
}

void tds_sound_source_set_pos(struct tds_sound_source* ptr, float x, float y) {
	alSource3f(ptr->source, AL_POSITION, x, y, 0.0f);
}

void tds_sound_source_set_vel(struct tds_sound_source* ptr, float x, float y) {
	alSource3f(ptr->source, AL_VELOCITY, x, y, 0.0f);
}

void tds_sound_source_set_vol(struct tds_sound_source* ptr, float vol) {
	alSourcef(ptr->source, AL_GAIN, vol);
}

void tds_sound_source_set_loop(struct tds_sound_source* ptr, int loop) {
	alSourcei(ptr->source, AL_LOOPING, loop);
}

void tds_sound_source_load_buffer(struct tds_sound_source* ptr, struct tds_sound_buffer* buf) {
	alSourceStop(ptr->source);
	alSourcei(ptr->source, AL_BUFFER, buf->buffer_id);
}

void tds_sound_source_play(struct tds_sound_source* ptr) {
	alSourcePlay(ptr->source);
}

void tds_sound_source_stop(struct tds_sound_source* ptr) {
	alSourceStop(ptr->source);
}
