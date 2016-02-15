#pragma once

struct IAudioClient2;
struct IAudioRenderClient;
struct AudioEngine
{
	IAudioClient2 *renderDevice;
	IAudioRenderClient *renderClient;
	uint32 frameSize;
};

struct Note
{
	uint32 sampleCount;
	uint32 samplesPerPeriod;
	int8 *data;
};

namespace Audio
{
	float GetMagnitudeForSample(uint32 samplePosition, uint32 totalSampleCount, float attackLevel);
	void InitNote(Note *note, uint32 frequency);
	void InitAudio(AudioEngine *engine);
}