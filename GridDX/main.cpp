#include "main.h"
#include "Graphics.h"
#include <ppltasks.h>
#include <audioclient.h>
#include <phoneaudioclient.h>

void DrawDebugData(Context *context, uint32 totalDataSize);
uint32 FrequencyFromIndex[10] = {262, 311, 349, 392, 466, 524, 622, 698, 698, 698};

float EaseInOutQuadraticParameter(float t)
{
	t = Math::Clamp(t, (float)0, (float)1);
	if (t < 0.5f)
	{
		return 2 * t * t;
	} 
	t--;
	return 1 - 2 * t * t;
}

void InitGraphics(Context *context)
{
	Vector2 screenSize = context->screenSize;
	Graphics::InitRenderer(&context->renderer);
	Graphics::InitRenderPass(&context->renderer, &context->quadPass, L"QuadVertexShader.cso", L"QuadPixelShader.cso", 
							 screenSize.y, screenSize.x);
	Graphics::InitRenderPass(&context->renderer, &context->spritePass, L"SpriteVertexShader.cso", L"SpritePixelShader.cso", 
							 screenSize.y, screenSize.x);
	Graphics::InitRenderPass(&context->renderer, &context->finalPass, L"SpriteVertexShader.cso", L"GrainPixelShader.cso", 0, 0);


	LPCWSTR pixelShaders[] = { L"BloomHorizontalPixelShader.cso", L"BloomVerticalPixelShader.cso", 
								L"GrainPixelShader.cso"};
	RenderPass *currentPass = &context->renderPipeline[0];
	for (uint32 i = 0; i < ARRAYSIZE(context->renderPipeline); i++)
	{
		Graphics::InitRenderPass(&context->renderer, currentPass, L"SpriteVertexShader.cso", pixelShaders[i],
								 screenSize.y, screenSize.x);
		currentPass++;
	}

	Audio::InitAudio(&context->audioEngine);

	Graphics::LoadTexture(&context->renderer, &context->tex, "rect","Assets/","dds");
	Graphics::LoadTexture(&context->renderer, &context->clearButton, "clear","Assets/","dds");
}

uint32 WrapIndex(uint32 value, uint32 offset, uint32 count)
{
	int32 newVal = (int32)value + (int32)offset;
	if (newVal < 0)
	{
		return newVal + count;
	}
	else if (newVal >= (int32)count)
	{
		return newVal - count;
	}
	return newVal;
}

uint32 GetBufferSizeInBytes(AudioBuffer *buffer)
{
	uint32 result = buffer->bytesPerSample * buffer->sampleRate * buffer->channels * buffer->seconds;
	return result;
}

uint32 GetBufferSampleCount(AudioBuffer *buffer)
{
	uint32 result = buffer->sampleRate * buffer->seconds;
	return result;
}


Note *GetNoteFromIndex(Palette *palette, uint32 index)
{ 
	Note *result = NULL;
	switch (index)
	{
		case 0:
			result = &palette->C;
			break;
		case 1:
			result = &palette->Eb;
			break;
		case 2:
			result = &palette->F;
			break;
		case 3:
			result = &palette->G;
			break;
		case 4:
			result = &palette->Bb;
			break;
		case 5:
			result = &palette->C2;
			break;
		case 6: 
			result = &palette->Eb2;
			break;
		case 7: 
			result = &palette->F2;
			break;
		case 8:
			result = &palette->G2;
			break;
		case 9:
		default:
			result = &palette->Bb2;
			break;

	}
	return result;
}

Matrix4x4 GetPortraitToLandscape(float screenHeight)
{
	Matrix4x4 rotation = Math::GetRotation(PIHALF, Vector3(0, 0, -1));
	Matrix4x4 translation = Math::GetTranslation(0, screenHeight, 0);
	Matrix4x4 result = translation * rotation;
	return result;
}

#define DEFAULT_VOLUME (0.8f)

void InitGameContext(Context *gameContext)
{
	Palette *palette = &gameContext->palette;
	for (int i = 0; i < ARRAYSIZE(gameContext->touchButtons); i++)
	{
		gameContext->touchButtons[i].value = 1;
		gameContext->touchButtons[i].playing = false;
		gameContext->touchButtons[i].glow = 1.0f;
	}

	for (int i = 0; i < ARRAYSIZE(gameContext->sounds); i++)
	{
		gameContext->sounds[i].dataPointer = 0;
		gameContext->sounds[i].attackLevel = 0;
		gameContext->sounds[i].note = GetNoteFromIndex(palette, i % 10);
		gameContext->sounds[i].playing = false;
		gameContext->sounds[i].delay = 0;
		gameContext->sounds[i].currentSample = 0;
		gameContext->sounds[i].previousVolume = 0;
		gameContext->sounds[i].currentVolume = 1.0f;

	}

	Audio::InitNote(&palette->C, 262);
	Audio::InitNote(&palette->Eb, 311);
	Audio::InitNote(&palette->F, 349);
	Audio::InitNote(&palette->G, 392);
	Audio::InitNote(&palette->Bb, 466);
	Audio::InitNote(&palette->C2, 523);
	Audio::InitNote(&palette->Eb2, 622);
	Audio::InitNote(&palette->F2, 698);
	Audio::InitNote(&palette->G2, 784);
	Audio::InitNote(&palette->Bb2, 932);
		
	HANDLE heap = GetProcessHeap();
	gameContext->audioBuffer.bytesPerSample = 2;
	gameContext->audioBuffer.channels = 2;
	gameContext->audioBuffer.sampleRate = 44100;
	gameContext->audioBuffer.seconds= 3;
	uint32 bufferByteSize = GetBufferSizeInBytes(&gameContext->audioBuffer);
	gameContext->audioBuffer.data = (uint8 *)HeapAlloc(heap, 0, bufferByteSize);
	gameContext->time = 0;
	gameContext->samplesErased = 0;
	gameContext->samplesSubmitted = 0;

	gameContext->debugPoints = (Vector2 *)HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(Vector2) * 10000);
	gameContext->isInitialized = true;

	gameContext->timeSinceClearButtonPressed = 1.0f;
}

void Update(Context *gameContext, GameInput *input, float dt)
{
	Renderer *renderer = &gameContext->renderer;
	ID3D11DeviceContext *context = renderer->context;
	if (!gameContext->isInitialized)
	{
		InitGameContext(gameContext);
	}

	gameContext->time += dt;

	Graphics::UpdateBlendState(renderer);

	float sceneWidth = 853;
	float sceneHeight = 480;
	uint32 xCount = XCOUNT;
	uint32 yCount = YCOUNT;
 
	Matrix4x4 screenProjection = 
		Math::GetOrthographicsProjectionRH(0, gameContext->screenSize.x, 0, gameContext->screenSize.y, -5.0f, 5.0f) * 
		GetPortraitToLandscape(gameContext->screenSize.y);
	Matrix4x4 sceneProjection = 
		Math::GetOrthographicsProjectionRH(0, sceneWidth, 0, sceneHeight, -5.0f, 5.0f);
	Matrix4x4 passProjection = 
		Math::GetOrthographicsProjectionRH(0, gameContext->screenSize.y, 0, gameContext->screenSize.x, -5.0f, 5.0f);
	Matrix4x4 viewIdentity = Math::GetIdentity();
	Graphics::SetColumnMajorProjectionView(renderer, &gameContext->finalPass, screenProjection, viewIdentity);
	Graphics::SetColumnMajorProjectionView(renderer, &gameContext->quadPass, sceneProjection, viewIdentity);
	Graphics::SetColumnMajorProjectionView(renderer, &gameContext->spritePass, sceneProjection, viewIdentity);
	RenderPass *currentPass = &gameContext->renderPipeline[0];
	for (uint32 i = 0; i < ARRAYSIZE(gameContext->renderPipeline); i++)
	{
		Graphics::SetColumnMajorProjectionView(renderer, currentPass, passProjection, viewIdentity);
		currentPass++;		
	}

    UINT32 bufferFrameCount;
    UINT32 numFramesPadding;
    UINT32 numFramesAvailable;
    BYTE *pData = NULL;
	
	uint32 chunkSize = 1500;
	AudioEngine *engine = &gameContext->audioEngine;
	AudioBuffer *audioBuffer = &gameContext->audioBuffer;

	HRESULT hr = engine->renderDevice->GetBufferSize(&bufferFrameCount);
	hr = engine->renderDevice->GetCurrentPadding(&numFramesPadding);
	numFramesAvailable = bufferFrameCount - numFramesPadding;
	gameContext->samplesNotPlayed -= gameContext->samplesNotPlayed - numFramesPadding;

	uint32 samplesPlayed = gameContext->samplesSubmitted - gameContext->samplesNotPlayed;
	uint32 samplesInCycle = GetBufferSampleCount(audioBuffer);
	uint32 totalDataSize = GetBufferSizeInBytes(audioBuffer);
	uint32 currentSampleCursor = (samplesPlayed - chunkSize * 100) % samplesInCycle;
	uint32 currentWriteCursor = (gameContext->samplesSubmitted) % samplesInCycle;
	uint32 currentPlayCursor = (gameContext->samplesSubmitted + chunkSize) % samplesInCycle;
	uint32 cursorPosition = (currentPlayCursor) / (samplesInCycle / xCount);
	
	uint32 samplesToErase = samplesPlayed - gameContext->samplesErased;
	uint32 firstChunkToEraseSizeInBytes = samplesToErase * engine->frameSize;
	int32 secondChunkToEraseSizeInBytes = firstChunkToEraseSizeInBytes - currentSampleCursor * engine->frameSize;
	if (secondChunkToEraseSizeInBytes > 0)
	{
		firstChunkToEraseSizeInBytes -= secondChunkToEraseSizeInBytes;
	}
	memset(audioBuffer->data + currentSampleCursor * engine->frameSize - firstChunkToEraseSizeInBytes, 0, 
		   firstChunkToEraseSizeInBytes);
	if (secondChunkToEraseSizeInBytes > 0)
	{
		memset(audioBuffer->data + totalDataSize - secondChunkToEraseSizeInBytes, 0, secondChunkToEraseSizeInBytes);
	}
	gameContext->samplesErased += samplesToErase;

	Button *b = &gameContext->touchButtons[0];
	int32 playCount = 0;
	for (uint32 x = 0; x < xCount; x++)
	{
		for (uint32 y = 0; y < yCount; y++)
		{
			if (cursorPosition == x && b->value == 0 && !b->playing)
			{
				Sound *sound = &gameContext->sounds[y];
				uint32 sampleToStart = (samplesInCycle / xCount) * x;
				sound->delay = (sampleToStart - currentWriteCursor);
				if (sound->delay < 0)
				{
					sound->delay += samplesInCycle;
				}
				if (sound->playing)
				{
					sound->attackLevel = Audio::GetMagnitudeForSample(sound->currentSample, sound->note->sampleCount, 0.0f);
					sound->attackLevel *= sound->currentVolume;
					sound->previousVolume = sound->currentVolume;
					sound->currentVolume = 1.0f;
				}
				sound->playing = true;
				b->playing = true;
			}
			else if (cursorPosition != x)
			{
				b->playing = false;
			}
			
			if (cursorPosition == x && b->value == 0)
			{
				playCount++;
			}
			b++;
		}
	}

	Sound *sound = &gameContext->sounds[0];
	for (uint32 i = 0; i < ARRAYSIZE(gameContext->sounds); i++)
	{
		sound->dataPointer = (gameContext->samplesSubmitted % sound->note->sampleCount) * engine->frameSize;
		sound++;
	}

	hr = engine->renderClient->GetBuffer(chunkSize, &pData);
	if (SUCCEEDED(hr))
	{
		if (gameContext->samplesNotPlayed < chunkSize)
		{
			uint32 bytesToWrite = chunkSize * engine->frameSize;
			uint32 byteToWrite = currentWriteCursor * engine->frameSize;
			uint32 played = 0;
			for (uint32 soundIndex = 0; soundIndex < yCount; soundIndex++)
			{
				Sound *sound = &gameContext->sounds[soundIndex];
				if (sound->playing)
				{
					if (sound->currentVolume == 1.0f)
					{
						float currentPlayCount = playCount;
						if (playCount > 1)
						{
							currentPlayCount *= 0.6f;
						}

						sound->currentVolume = DEFAULT_VOLUME / currentPlayCount;
						sound->attackLevel /= sound->currentVolume;
					}
					float volume = sound->currentVolume;
					if (sound->delay > 0)
					{
						volume = sound->previousVolume;
					}

					uint32 noteBytes = sound->note->sampleCount * engine->frameSize;
					int32 sourceDataFirstChunkBytes = bytesToWrite;
					int32 sourceDataSecondChunkBytes = sound->dataPointer + sourceDataFirstChunkBytes - noteBytes;
					if (sourceDataSecondChunkBytes > 0)
					{
						sourceDataFirstChunkBytes -= sourceDataSecondChunkBytes;
					}
					int16 *sourceFirstSample = (int16 *)(sound->note->data + sound->dataPointer);

					int32 destDataFirstChunkBytes = bytesToWrite;
					int32 destDataSecondChunkBytes = byteToWrite + destDataFirstChunkBytes - totalDataSize;
					if (destDataSecondChunkBytes > 0)
					{
						destDataFirstChunkBytes -= destDataSecondChunkBytes;
					}
					int16 *destFirstSample = (int16 *)(audioBuffer->data + byteToWrite);

					int32 byteDelay = sound->delay * engine->frameSize;
					for (uint32 i = 0; i < bytesToWrite; i += 2)
					{
						if (i == sourceDataFirstChunkBytes)
						{
							sourceFirstSample = (int16 *)sound->note->data;
						}
						if (i == destDataFirstChunkBytes)
						{
							destFirstSample = (int16 *)audioBuffer->data;
						}
						float magnitude = Audio::GetMagnitudeForSample(
							sound->currentSample, 
							sound->note->sampleCount, sound->attackLevel);
						*destFirstSample++ += (int16)(*(sourceFirstSample++) * magnitude * volume);
						if (i % engine->frameSize == 2)
						{
							sound->currentSample++;
						}
						if (byteDelay > 0 && i + 2 == byteDelay)
						{
							sound->currentSample = 0;
							volume = sound->currentVolume;
						}
					}
					byteDelay -= bytesToWrite;
					sound->delay = byteDelay / (int32)engine->frameSize;
					if (sound->delay < 0)
					{
						sound->delay = 0;
					}
					if (sound->currentSample >= sound->note->sampleCount)
					{
						sound->currentSample = 0;
						sound->delay = 0;
						sound->playing = false;
						sound->attackLevel = 0;
						sound->currentVolume = 1.0f;
						sound->previousVolume = 0.0f;
					}
				}
			}

			int32 firstWriteSize = bytesToWrite;
			int32 secondaryWriteSize = byteToWrite + bytesToWrite - totalDataSize;
			if (secondaryWriteSize > 0)
			{
				firstWriteSize -= secondaryWriteSize;
			}
			memcpy(pData, audioBuffer->data + byteToWrite, firstWriteSize);
			if (secondaryWriteSize > 0)
			{
				memcpy(pData + firstWriteSize, audioBuffer->data, secondaryWriteSize);
			}
			gameContext->samplesSubmitted += bytesToWrite / engine->frameSize;
			gameContext->samplesNotPlayed += bytesToWrite / engine->frameSize;
			hr = engine->renderClient->ReleaseBuffer(bytesToWrite / engine->frameSize, 0);
		}
		else
		{
			hr  = engine->renderClient->ReleaseBuffer(0, AUDCLNT_BUFFERFLAGS_SILENT);
		}
	}
	
	float rectSize = 400.0f / (float)yCount;
	float totalWidth = xCount * rectSize;
	float horizontalSpace = sceneWidth - totalWidth;
	Vector2 clearButtonSize = Vector2(gameContext->clearButton.width, gameContext->clearButton.height);
	Vector2 clearButtonPosition = Vector2(sceneWidth - horizontalSpace / 2.0f + 5.0f, sceneHeight);

	for (uint32 i = 0; i < input->touchCount; i++)
	{
		Vector2 touchPosition = input->touches[i];
		if (touchPosition.x < clearButtonPosition.x && touchPosition.x > clearButtonPosition.x - clearButtonSize.x &&
			touchPosition.y < clearButtonPosition.y && touchPosition.y > clearButtonPosition.y - clearButtonSize.y)
		{
			Button *button = &gameContext->touchButtons[0];
			for (uint32 i = 0; i < ARRAYSIZE(gameContext->touchButtons); i++)
			{
				button++->value = 1;
			}
			gameContext->timeSinceClearButtonPressed = 0.0f;
		}
		touchPosition.x -= horizontalSpace / 2.0f;
		float touchPanelWidth = xCount * rectSize;
		float touchPanelHeight = yCount * rectSize;
		if (touchPosition.x >= 0 && touchPosition.x < touchPanelWidth &&
			touchPosition.y >= 0 && touchPosition.y < touchPanelHeight)
		{
			uint32 buttonPressedX = (uint32)(touchPosition.x / rectSize);
			uint32 buttonPressedY = (uint32)(touchPosition.y / rectSize);

			if (gameContext->touchButtons[buttonPressedY + buttonPressedX * yCount].value == 0)
				gameContext->touchButtons[buttonPressedY + buttonPressedX * yCount].value = 1;
			else														
				gameContext->touchButtons[buttonPressedY + buttonPressedX * yCount].value = 0;
		}
	}

	Graphics::StartPass(renderer, &gameContext->spritePass, Color(0.0f, 0.0f, 0.0f, 1.0f));
	Vector2 clearButtonOrigin = Vector2(1,1);
	float clearGlow = EaseInOutQuadraticParameter(gameContext->timeSinceClearButtonPressed);
	Color clearButtonColor = Color::ByteColor(255, 155, 0) * (1.5f - 0.5f * clearGlow);
	if (gameContext->timeSinceClearButtonPressed < 1.0f)
	{
		gameContext->timeSinceClearButtonPressed += dt;
		if (gameContext->timeSinceClearButtonPressed > 1.0f)
		{
			gameContext->timeSinceClearButtonPressed = 1.0f;
		}
	}

	Graphics::RenderTexture(renderer, &gameContext->spritePass, &gameContext->clearButton, clearButtonPosition, 
							clearButtonOrigin, clearButtonColor, 1.0f);

	Vector2 rectPosition = Vector2(horizontalSpace / 2.0f,0);
	float rectPaddingSize = rectSize / 10.0f;
	Vector2 rectPadding = Vector2(rectPaddingSize, rectPaddingSize);
	float rectFillSize = rectSize - 2.0f * rectPaddingSize;
	float scale = rectFillSize / 100.0f;
	Vector2 rectOrigin = Vector2(rectFillSize / 2.0f, rectFillSize / 2.0f);
	
	for (uint32 y = 0; y < yCount; y++)
	{
		Vector2 currentRow = rectPosition;
		for (uint32 x = 0; x < xCount; x++)
		{
			Button *button = &gameContext->touchButtons[y + x * yCount];
			uint32 value = button->value;
			Color buttonColor = Color::ByteColor(0, 140, 255) * (1.0f + y * 0.066f);//Color(0.6f, 0.6f, 1.0f, 1);
			if (value > 0)
			{
				buttonColor = Color(0.4f, 0.4f, 0.4f, 1);
			}
			if (cursorPosition == x && button->glow == 1.0f)
			{
				button->glow = 2.0f;
			}
			float glowPart = button->glow - 1.0f;
			float glow = EaseInOutQuadraticParameter(glowPart) + 1.0f;
			buttonColor = buttonColor * glow;
			buttonColor.a = Math::Clamp(buttonColor.a, 0.0f,1.0f);
			if (button->glow > 1.0f)
			{
				button->glow -= dt;
				if (button->glow < 1.0f)
				{
					button->glow = 1.0f;
				}
			} 

			Graphics::RenderTexture(renderer, &gameContext->spritePass, &gameContext->tex, currentRow + rectPadding + rectOrigin, 
									Vector2(0.5f, 0.5f), buttonColor, scale);
			
			currentRow.x += rectSize;
		}
		rectPosition.y += rectSize;
	}

	currentPass = &gameContext->renderPipeline[0];
	RenderPass *previousPass = &gameContext->spritePass;
	/*for (uint32 i = 0; i < ARRAYSIZE(gameContext->renderPipeline); i++)
	{
		Graphics::StartPass(renderer, currentPass, Color(0,0,0,0));
		Graphics::RenderTexture(renderer, currentPass, &previousPass->renderTargetTexture,
								Vector2(0, 0), Vector2(0, 0), Color(1,1,1,1), 1.0f);
		previousPass = currentPass++;
	}*/

	Vector2 screenCenter = Vector2(gameContext->screenSize.y, gameContext->screenSize.x) / 2.0f;
	Graphics::StartPass(renderer, &gameContext->finalPass, Color(0,0,0,0));
	Graphics::RenderTexture(renderer, &gameContext->finalPass, &gameContext->spritePass.renderTargetTexture, 
							Vector2(0, 0), Vector2(0, 0), Color(1,1,1,1), 1.0f);
	Graphics::RenderTexture(renderer, &gameContext->finalPass, &previousPass->renderTargetTexture, 
							Vector2(0, 0), Vector2(0, 0), Color(1,1,1,1.0f), 1.0f);
	//DrawDebugData(gameContext, totalDataSize);
}

void DrawDebugData(Context *gameContext, uint32 totalDataSize)
{
	ID3D11DeviceContext *context = gameContext->renderer.context;
	float totalWidth = gameContext->screenSize.y;
	float totalHeight = 200;
	Vector2 *start = gameContext->debugPoints;
	uint32 stride = totalDataSize / 20000;
	float stepSizeX = totalWidth / 10000;
	int16 *audioData = (int16 *)gameContext->audioBuffer.data;
	for (int i = 0; i < 10000; i++)
	{
		start->x = i * stepSizeX;
		float data = (float)audioData[stride * i];
		start->y = (data / 8000) * totalHeight / 2.0f + totalHeight / 2.0f;
		start++;
	}

	D3D11_MAPPED_SUBRESOURCE resource;
    context->Map(gameContext->renderer.debugPass.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    memcpy(resource.pData, &gameContext->debugPoints[0], sizeof(Vector2) * 10000);
    context->Unmap(gameContext->renderer.debugPass.vertexBuffer, 0);

	Graphics::DrawDebugData(&gameContext->renderer, 10000);
}

#if 0
uint32 byteToStart = currentWriteCursor * engine->frameSize;// - (sound->dataPointer) / engine->frameSize;
					//uint32 sampleToStart = (44100 * 5 * sound->position) / 16;
					if (playCount > 1)
						sound->volume = DEFAULT_VOLUME / playCount;
					else
						sound->volume = DEFAULT_VOLUME;
					uint32 sizeToWriteInBytes = chunkSize * engine->frameSize;
					uint32 bufferTotalSize = sound->note->sampleCount * engine->frameSize;
					if (sound->dataPointer + sizeToWriteInBytes > bufferTotalSize)
					{
						sizeToWriteInBytes = bufferTotalSize - sound->dataPointer;
					}
					bytesToWrite = Math::Max(sizeToWriteInBytes, bytesToWrite);

					int16 *samplePointer = (int16 *)(gameContext->audioData + byteToStart);
					if (byteToStart > totalDataSize)
					{
						samplePointer -= totalDataSize / 2;
					}

					uint32 firstChunkSizeInBytes = sizeToWriteInBytes;
					int32 secondChunkSizeInBytes = (byteToStart + sizeToWriteInBytes) - totalDataSize;
					int32 secondChunkOffset = secondChunkSizeInBytes - sizeToWriteInBytes;
					if (secondChunkOffset < 0)
					{
						secondChunkOffset = 0;
					}
					secondChunkSizeInBytes = Math::Clamp(secondChunkSizeInBytes, 0, sizeToWriteInBytes);
					if (secondChunkSizeInBytes > 0)
					{
						firstChunkSizeInBytes -= secondChunkSizeInBytes;
					}
						
					static int16 maxAmplitude = 32767.0f / 3.0f;
					static int16 threshold = 32767.0f / 4.0f * 0.f;
					int16 *currentSample = (int16 *)(sound->note->data + sound->dataPointer);
					for (uint32 sample = 0; sample < firstChunkSizeInBytes / 2; sample++)
					{
						int32 newValue = *samplePointer;
						int16 currentValue = *currentSample;
						float attackLevel = 0;
						if (sound->secondaryOffset > 0)
						{
							attackLevel = Audio::GetMagnitudeForSample(
								(sound->secondaryOffset) / engine->frameSize , sound->note->sampleCount, 0);
							
						} 
						float sampleMagnitude = Audio::GetMagnitudeForSample(
							(sound->dataPointer) / engine->frameSize + sample / 2, 
							sound->note->sampleCount, attackLevel);
						newValue += (currentValue) * sound->volume * sampleMagnitude;
						*samplePointer++ = (int16)newValue;
						currentSample++;
					}
					if (secondChunkSizeInBytes > 0)
					{
						samplePointer = (int16 *)(gameContext->audioData + secondChunkOffset);
						for (uint32 sample = 0; sample < secondChunkSizeInBytes / 2; sample++)
						{
							int32 newValue = *samplePointer;
							int16 currentValue = *currentSample;
							float attackLevel = 0;
							if (sound->secondaryOffset > 0)
							{
								attackLevel = Audio::GetMagnitudeForSample(
									(sound->secondaryOffset) / engine->frameSize, sound->note->sampleCount, 0);
								
							} 
							float sampleMagnitude = Audio::GetMagnitudeForSample(
								(sound->dataPointer + firstChunkSizeInBytes) / engine->frameSize + sample / 2, 
								sound->note->sampleCount, attackLevel);
							newValue += (currentValue) * sound->volume * sampleMagnitude; 
							*samplePointer++ = (int16)newValue;
							currentSample++;
						}
					}

					sound->dataPointer += sizeToWriteInBytes;
					if (sound->dataPointer == bufferTotalSize)
					{
						sound->playing = false;
						sound->dataPointer = 0;
						sound->volume = 1.0f;
						sound->secondaryOffset = 0;
					}
#endif

#if 0
			for (uint32 x = 0; x < xCount; x++)
			{
				uint32 sampleToStart = (44100 * 5 * x) / 16;
				for (uint32 y = 0; y < yCount; y++)
				{
					if (button->playing)
					{
						if (button->targetVolume == 1.0f)
						{
							button->targetVolume = Math::Clamp(1.0f / (polyCoefs[x] + polyCoefs[x == 0 ? 15 : x-1]), 0.0f, .8f);
							if (playCount == 1)
							{
								button->targetVolume = DEFAULT_VOLUME;
							}
						}
						if (button->volume < button->targetVolume)
						{
							button->volume += dt * 2.0f;
							if (button->volume > button->targetVolume)
							{
								button->volume = button->targetVolume;
							}
						}
						else
						{
							button->volume -= dt * 2.0f;
							if (button->volume < button->targetVolume)
							{
								button->volume = button->targetVolume;
							}
						}
						button->volume = button->targetVolume;
						played++;
						uint32 sizeToWriteInBytes = chunkSize * engine->frameSize;
						uint32 bufferTotalSize = button->note->sampleCount * engine->frameSize;
						if (button->dataPointer + sizeToWriteInBytes > bufferTotalSize)
						{
							sizeToWriteInBytes = bufferTotalSize - button->dataPointer;
						}
						bytesToWrite = Math::Max(sizeToWriteInBytes, bytesToWrite);

						int16 *samplePointer = (int16 *)(gameContext->audioData + sampleToStart * engine->frameSize + 
														 button->dataPointer);
						if (sampleToStart * engine->frameSize + button->dataPointer > totalDataSize)
						{
							samplePointer -= totalDataSize / 2;
						}

						uint32 firstChunkSizeInBytes = sizeToWriteInBytes;
						int32 secondChunkSizeInBytes = (sampleToStart * engine->frameSize + button->dataPointer + 
														sizeToWriteInBytes) - totalDataSize;
						int32 secondChunkOffset = secondChunkSizeInBytes - sizeToWriteInBytes;
						if (secondChunkOffset < 0)
						{
							secondChunkOffset = 0;
						}
						secondChunkSizeInBytes = Math::Clamp(secondChunkSizeInBytes, 0, sizeToWriteInBytes);
						if (secondChunkSizeInBytes > 0)
						{
							firstChunkSizeInBytes -= secondChunkSizeInBytes;
						}
						
						static int16 maxAmplitude = 32767.0f / 3.0f;
						static int16 threshold = 32767.0f / 4.0f * 0.f;
						int16 *currentSample = (int16 *)(button->note->data + button->dataPointer);
						for (uint32 sample = 0; sample < firstChunkSizeInBytes / 2; sample++)
						{
							int32 newValue = *samplePointer;
							newValue += (*currentSample) * button->volume;
							if (played == playCount && Math::Abs((int16)newValue) > threshold)
							{
								//newValue = Compress(newValue, maxAmplitude, threshold, playCount);
							}
							*samplePointer++ = (int16)newValue;
							currentSample++;
						}
						if (secondChunkSizeInBytes > 0)
						{
							samplePointer = (int16 *)(gameContext->audioData + secondChunkOffset);
							for (uint32 sample = 0; sample < secondChunkSizeInBytes / 2; sample++)
							{
								int32 newValue = *samplePointer;
								newValue += (*currentSample) * button->volume;
								if (played == playCount && Math::Abs((int16)newValue) > threshold)
								{
									//newValue = Compress(newValue, maxAmplitude, threshold, playCount);
								}
								*samplePointer++ = (int16)newValue;
								currentSample++;
							}
						}

						button->dataPointer += sizeToWriteInBytes;
						if (button->dataPointer == bufferTotalSize)
						{
							button->playing = false;
							button->dataPointer = 0;
							button->volume = DEFAULT_VOLUME;
							button->targetVolume = 1.0f;
						}
					}
					button++;
				}
			}
#endif