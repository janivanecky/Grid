#pragma once
#include "Graphics.h"
#include "Audio.h"

struct Button
{
	uint32 value;
	bool playing;
	float glow;
};

struct Sound
{
	Note *note;
	uint32 currentSample;
	uint32 dataPointer;
	int32 delay;
	float previousVolume;
	float currentVolume;
	float attackLevel;
	bool playing;
};

struct Palette
{
	Note C;
	Note Eb;
	Note F;
	Note G;
	Note Bb;
	Note C2;
	Note Eb2;
	Note F2;
	Note G2;
	Note Bb2;
};

struct AudioBuffer
{
	uint8 *data;
	uint32 sampleRate;
	uint32 seconds;
	uint32 channels;
	uint32 bytesPerSample;
};

#define XCOUNT 20
#define YCOUNT 10
struct Context
{
	bool isInitialized;

	float time;

	Renderer renderer;
	AudioEngine audioEngine;
	AudioBuffer audioBuffer;

	Button touchButtons[XCOUNT * YCOUNT];
	Sound sounds[10];

	uint32 samplesSubmitted;
	uint32 samplesNotPlayed;
	uint32 samplesErased;

	Palette palette;
	Texture tex;
	float timeSinceClearButtonPressed;
	Texture clearButton;

	Vector2 *debugPoints;
	
	Vector2 screenSize;
	RenderPass spritePass;
	RenderPass quadPass;
	RenderPass finalPass;

	RenderPass renderPipeline[1];
};


struct GameInput
{
	uint32 touchCount;
	Vector2 touches[5];
};

void InitGraphics(Context *context);
void Update(Context *context, GameInput *input, float dt);