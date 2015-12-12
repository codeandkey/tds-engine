#include "sound_buffer.h"
#include "memory.h"
#include "log.h"

#include <AL/al.h>

#include "libs/stb_vorbis.h"

static unsigned int _tds_sound_buffer_load(char* filename);

struct tds_sound_buffer* tds_sound_buffer_create(char* filename) {
	struct tds_sound_buffer* output = tds_malloc(sizeof(struct tds_sound_buffer));

	output->buffer_id = _tds_sound_buffer_load(filename);

	tds_logf(TDS_LOG_DEBUG, "Loaded sound [%s] into buffer id %d.\n", filename, output->buffer_id);

	return output;
}

void tds_sound_buffer_free(struct tds_sound_buffer* ptr) {
	alDeleteBuffers(1, &ptr->buffer_id);
	tds_free(ptr);
}

unsigned int _tds_sound_buffer_load(char* filename) {
	short* pcm_data; // PCM data is 16-bit signed.
	int channels = 1, freq = 48000;
	int pcm_data_length = stb_vorbis_decode_filename(filename, &channels, &freq, &pcm_data);

	if (pcm_data_length <= 0) {
		tds_logf(TDS_LOG_CRITICAL, "Failed to decode file [%s].\n", filename);
		return 0;
	}

	unsigned int output = 0;
	alGenBuffers(1, &output);

	if (!alIsBuffer(output)) {
		tds_logf(TDS_LOG_CRITICAL, "OpenAL buffer generation failed.\n");
		return 0;
	}

	unsigned int format = ((channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16);

	tds_logf(TDS_LOG_DEBUG, "Format: %d channels, %d bits per sample, %d frequency, %d total size\n", channels, 16, freq, pcm_data_length);

	alBufferData(output, format, pcm_data, pcm_data_length * channels * 2, freq);

	ALenum er = alGetError();

	if (er != AL_NO_ERROR) {
		tds_logf(TDS_LOG_WARNING, "OpenAL error : %d\n", er);
	}

	return output;
}
