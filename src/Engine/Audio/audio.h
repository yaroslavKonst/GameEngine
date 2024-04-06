#ifndef _AUDIO_H
#define _AUDIO_H

#include <vector>
#include <mutex>
#include <atomic>
#include <portaudio.h>

#include "../Utils/RingBuffer.h"

class Audio
{
public:
	static constexpr int SampleRate = 44100;

	struct Buffer
	{
		std::vector<float> Data;
		float Multiplier;
		std::atomic<bool> Finished;
		bool Active;
		bool Discard;

		Buffer()
		{
			Multiplier = 1.0f;
			Finished = false;
			Active = true;
			Discard = false;
		}
	};

	Audio();
	~Audio();

	void Submit(Buffer* buffer);

private:
	struct BufferData
	{
		Buffer* buffer;
		size_t position;

		BufferData* next;

		BufferData()
		{
			buffer = nullptr;
			position = 0;
			next = nullptr;
		}
	};

	PaStream* _stream;

	BufferData* _activeBuffers;
	BufferData* _activeBuffersEnd;

	RingBuffer<BufferData*> _inBuffers;
	RingBuffer<BufferData*, false> _outBuffers;

	static int AudioCallback(
		const void* inputBuffer,
		void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);
};

#endif
