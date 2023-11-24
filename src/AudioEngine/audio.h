#ifndef _AUDIO_H
#define _AUDIO_H

#include <vector>
#include <mutex>
#include <portaudio.h>

class Audio
{
public:
	struct Buffer
	{
		std::vector<float> Data;
		float Multiplier;
		bool Finished;
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
		bool valid;

		BufferData()
		{
			buffer = nullptr;
			position = 0;
			valid = false;
		}
	};

	PaStream* _stream;

	std::mutex _submitMutex;

	std::vector<BufferData> _ringBuffer;
	size_t _bufferStart;
	size_t _bufferEnd;

	static int AudioCallback(
		const void* inputBuffer,
		void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);
};

#endif
