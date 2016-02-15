#include "Audio.h"
#include "GridMath.h"
#include <audioclient.h>
#include <phoneaudioclient.h>

#if 0
void Audio::InitNote(AudioEngine *engine, Note *note, uint32 frequency, int32 startingMagnitude)
{
	float duration = 1.0f;
	uint32 sampleCount = (uint32)(44100.0f * duration);
	uint32 periodSamples = 44100 / frequency;
	uint32 periodCount = sampleCount / periodSamples;
	HANDLE heap = GetProcessHeap();
	sampleCount = periodSamples * periodCount;
	note->size = sampleCount * 2 * 2;
	note->data = (int16 *)HeapAlloc(heap, 0, note->size);
	int16 *p = note->data;
	float attack = 0.1f;
	float decay = 0.2f;
	float release = 0.6f;
	float sustain = 0.3f;
	for (uint32 i = 0; i < sampleCount; i++)
	{
		uint32 sampleInPeriod = i % periodSamples;
		float partInPeriod = ((float)sampleInPeriod / (float)periodSamples);// * PI2;
		float partInTone = ((float)i) / ((float)sampleCount);
		float c = partInTone;
		c = Math::Sin(partInTone * PI) * 32767.0f;
		c = Math::Cos(partInTone * PIHALF) * 32767.0f;
		//c = Math::ExponentialFunction(1, -partInTone * 2.5f, partInTone * 2.5f);

		/*float t = 1;
		if (partInTone < 0.4f)
		t = partInTone / (0.4f);
		else if (partInTone > 0.6f)
		t = (1.0f - partInTone) / (0.4f);
		float amplitude = Math::ExponentialFunction(1, 15, t) - 1;
		*///c = amplitude;

		float magnitude = 32767.0f / 1.0f;
		if (partInTone < attack)
		{
			c = startingMagnitude + (magnitude - startingMagnitude) * partInTone / attack;
		}
		else if (partInTone < decay + attack)
		{
			c = magnitude * sustain + magnitude * (1.0f - sustain) * (decay - (partInTone - attack)) / decay;
		}
		else if (partInTone > 1.0f - release)
		{
			c = (magnitude * sustain) * (release - (partInTone - (1.0f - release))) / release;
		}
		else
		{
			c = magnitude * sustain;
		}

		//*p++ = (int16)(Math::Sin(partInPeriod) * c);
		//*p++ = (int16)(Math::Sin(partInPeriod) * c);

		*p++ = (int16)(partInPeriod * c);
		*p++ = (int16)(partInPeriod * c);
	}

	note->buffer.Flags = 0;
	note->buffer.AudioBytes = note->size;
	note->buffer.pAudioData = (BYTE *)note->data;
	note->buffer.PlayBegin = 0;
	note->buffer.PlayLength = 0;
	note->buffer.LoopBegin = 0;
	note->buffer.LoopLength = 0;
	note->buffer.LoopCount = 0;
	note->buffer.pContext = NULL;
}
#endif

float Audio::GetMagnitudeForSample(uint32 samplePosition, uint32 totalSampleCount, float attackLevel)
{
	float attack = 0.02f;
	float decay = 0.08f;
	float release = 0.92f;
	float sustain = 0.06f;
	float magnitude = 1.0f;//32767.0f / 4.0f;
	float c = 0.0f;
	float partInTone = ((float)samplePosition) / ((float)totalSampleCount);
	if (partInTone < attack)
	{
		c = attackLevel + (1.0f - attackLevel) *(magnitude) * partInTone / attack;
	}
	else if (partInTone < decay + attack)
	{
		c = magnitude * sustain + magnitude * (1.0f - sustain) * (decay - (partInTone - attack)) / decay;
	}
	else if (partInTone > 1.0f - release)
	{
		c = (magnitude * sustain) * (release - (partInTone - (1.0f - release))) / release;
	}
	else
	{
		c = magnitude * sustain;
	}
	return c;
}

void Audio::InitNote(Note *note, uint32 frequency)
{
	float duration = 2.0f;
	uint32 totalSampleCount = (uint32)(44100.0f * duration);
	uint32 samplesPerPeriod = 44100 / frequency;
	uint32 periodCount = totalSampleCount / samplesPerPeriod;
	totalSampleCount = samplesPerPeriod * periodCount;
	note->sampleCount = totalSampleCount;
	note->samplesPerPeriod = samplesPerPeriod;
	HANDLE heap = GetProcessHeap();
	note->data = (int8 *)HeapAlloc(heap, 0, totalSampleCount * 2 * 2);
	int16 *p = (int16 *)note->data;
	for (uint32 i = 0; i < totalSampleCount; i++)
	{
		uint32 sampleInPeriod = i % samplesPerPeriod;
		float partInPeriod = ((float)sampleInPeriod / (float)(samplesPerPeriod)) * PI2;
		
		float magnitude = 32767.0f / 4.0f;
		float c = magnitude;//GetMagnitudeForSample(i, totalSampleCount);
		
		float sinPart = 1.0f;
		*p++ = (int16)(Math::Sin(partInPeriod) * c * sinPart) + (int16)(partInPeriod / PI2 * c * (1.0f - sinPart));
		*p++ = (int16)(Math::Sin(partInPeriod) * c * sinPart) + (int16)(partInPeriod / PI2 * c * (1.0f - sinPart));

	}
}


void Audio::InitAudio(AudioEngine *engine)
{
	WAVEFORMATEX *format = NULL;
	HRESULT hr = E_FAIL;
 
    LPCWSTR renderId = GetDefaultAudioRenderId(AudioDeviceRole::Default);
	if (renderId != NULL)
	{
        hr = ActivateAudioInterface(renderId, __uuidof(IAudioClient2), (void**)&engine->renderDevice);
		if (SUCCEEDED(hr))
		{
			hr = engine->renderDevice->GetMixFormat(&format);
		    AudioClientProperties properties = {};
			if (SUCCEEDED(hr))
			{
				properties.cbSize = sizeof AudioClientProperties;
				properties.eCategory = AudioCategory_Other;
				hr = engine->renderDevice->SetClientProperties(&properties);
				if (SUCCEEDED(hr))
				{
					WAVEFORMATEX temp;
					temp.cbSize = 0;
					temp.nChannels = 2;
					temp.wBitsPerSample = 16;
					temp.nSamplesPerSec = 44100;
					temp.nBlockAlign = temp.nChannels * temp.wBitsPerSample / 8;
					temp.nAvgBytesPerSec = temp.nBlockAlign * temp.nSamplesPerSec;
					temp.wFormatTag = WAVE_FORMAT_PCM;
 
					*format = temp;
					engine->frameSize = (format->wBitsPerSample / 8) * format->nChannels;
 
					hr = engine->renderDevice->Initialize(AUDCLNT_SHAREMODE_SHARED, 0x88000000, 1000 * 10000, 0, format, NULL);
					if (SUCCEEDED(hr))
					{
						hr = engine->renderDevice->GetService(__uuidof(IAudioRenderClient), (void**)&engine->renderClient);
						if (SUCCEEDED(hr))
						{
							hr = engine->renderDevice->Start();
						}
					}
				}
 
			}
		}
	}
 
    if (renderId)
	{
		CoTaskMemFree((LPVOID)renderId);
	}
}


