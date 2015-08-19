#pragma once

#include <AL/al.h>
#include <AL/alc.h>

struct tds_sound_manager {
	ALCdevice* device;
	ALCcontext* context;
	unsigned int source;
};

struct tds_sound_manager* tds_sound_manager_create(void);
void tds_sound_manager_free(struct tds_sound_manager* ptr);
