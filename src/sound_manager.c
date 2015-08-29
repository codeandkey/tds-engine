#include "sound_manager.h"
#include "log.h"
#include "memory.h"

#include <stdlib.h>

struct tds_sound_manager* tds_sound_manager_create(void) {
	struct tds_sound_manager* output = tds_malloc(sizeof(struct tds_sound_manager));

	output->device = alcOpenDevice(NULL);

	if (!output->device) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to open default audio device.\n");
		return NULL;
	}

	output->context = alcCreateContext(output->device, NULL);

	if (!alcMakeContextCurrent(output->context)) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to create audio context.\n");
		return NULL;
	}

	ALfloat ori[6] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

	alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
	alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alListenerfv(AL_ORIENTATION, ori);

	ALenum er = alGetError();

	if (er != AL_NO_ERROR) {
		tds_logf(TDS_LOG_WARNING, "OpenAL error : %d\n", er);
	}

	alDopplerFactor(4.0f);

	return output;
}

void tds_sound_manager_free(struct tds_sound_manager* ptr) {
	alcMakeContextCurrent(NULL);
	alcDestroyContext(ptr->context);
	alcCloseDevice(ptr->device);
	tds_free(ptr);
}

void tds_sound_manager_set_pos(struct tds_sound_manager* ptr, float x, float y) {
	alListener3f(AL_POSITION, x, y, 0.0f);
}
