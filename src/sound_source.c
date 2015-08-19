#include "sound_source.h"
#include "memory.h"

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
