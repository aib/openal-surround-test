#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <math.h>
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif
#define M_TAU (M_PI * 2.0)

#include <AL/al.h>
#include <AL/alc.h>

#define SAMPLE_RATE 44100
#define SINE_FREQ 440

#define ORBIT_PERIOD 3
#define ORBITS 10
#define ORBIT_DISTANCE 2.0f

int checkForAlErrors(void);

int main(void)
{
	int err;

	ALCdevice *dev = alcOpenDevice(NULL);
	if (!dev) {
		fprintf(stderr, "Unable to open default device\n");
		err = EXIT_FAILURE;
		goto exit;
	}

	ALCcontext *ctx = alcCreateContext(dev, NULL);
	if (!ctx) {
		fprintf(stderr, "Unable to create context\n");
		err = EXIT_FAILURE;
		goto exit_close_device;
	}

	alcMakeContextCurrent(ctx);
	if (checkForAlErrors()) {
		fprintf(stderr, "Unable to make context current\n");
		err = EXIT_FAILURE;
		goto exit_destroy_context;
	}

	ALuint buffers[1];
	alGenBuffers(sizeof(buffers) / sizeof(*buffers), buffers);
	if (checkForAlErrors()) {
		fprintf(stderr, "Unable to generate buffers\n");
		err = EXIT_FAILURE;
		goto exit_destroy_context;
	}

	ALuint sources[1];
	alGenSources(sizeof(sources) / sizeof(*sources), sources);
	if (checkForAlErrors()) {
		fprintf(stderr, "Unable to generate sources\n");
		err = EXIT_FAILURE;
		goto exit_delete_buffers;
	}

	int16_t sineData[SAMPLE_RATE / SINE_FREQ];
	for (size_t i = 0; i < SAMPLE_RATE / SINE_FREQ; ++i) {
		sineData[i] = sin(i * M_TAU * SINE_FREQ / SAMPLE_RATE) * INT16_MAX;
	}

	alBufferData(buffers[0], AL_FORMAT_MONO16, sineData, sizeof(sineData), SAMPLE_RATE);
	if (checkForAlErrors()) {
		fprintf(stderr, "Unable to set buffer data\n");
		err = EXIT_FAILURE;
		goto exit_delete_sources;
	}

	alSourcei(sources[0], AL_BUFFER, buffers[0]);
	if (checkForAlErrors()) {
		fprintf(stderr, "Unable to attach buffer to source\n");
		err = EXIT_FAILURE;
		goto exit_delete_sources;
	}

	alSourcei(sources[0], AL_LOOPING, AL_TRUE);

	alSourcePlay(sources[0]);
	if (checkForAlErrors()) {
		fprintf(stderr, "Unable to play source\n");
		err = EXIT_FAILURE;
		goto exit_delete_sources;
	}

	clock_t beginClock = clock();
	while (1) {
		double elapsedSec = (clock() - beginClock) / (double) CLOCKS_PER_SEC;
		double orbit = elapsedSec / ORBIT_PERIOD;

		if (orbit > ORBITS) break;

		alSource3f(
			sources[0],
			AL_POSITION,
			sin(orbit * M_TAU) * ORBIT_DISTANCE,
			0.0f,
			-cos(orbit * M_TAU) * ORBIT_DISTANCE
		);
	}

	err = EXIT_SUCCESS;

exit_delete_sources:
	alDeleteSources(sizeof(sources) / sizeof(*sources), sources);
exit_delete_buffers:
	alDeleteBuffers(sizeof(buffers) / sizeof(*buffers), buffers);
exit_destroy_context:
	alcDestroyContext(ctx);
exit_close_device:
	alcCloseDevice(dev);
exit:
	return err;
}

int checkForAlErrors(void)
{
	ALenum error = alGetError();
	if (error == AL_NO_ERROR) {
		return 0;
	} else {
		fprintf(stderr, "Error: %04X\n", error);
		return 1;
	}
}

