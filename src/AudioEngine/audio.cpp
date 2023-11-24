#include "audio.h"

#include <cstring>
#include <stdexcept>

#include "../Logger/logger.h"

#define RING_BUFFER_SIZE 1024 * 1024
#define SAMPLE_RATE 44100

Audio::Audio() : _ringBuffer(RING_BUFFER_SIZE)
{
	_bufferStart = 0;
	_bufferEnd = 0;

	PaError err = Pa_Initialize();

	if (err != paNoError) {
		throw std::runtime_error(
			std::string("Failed to initialize portaudio: ") +
			Pa_GetErrorText(err));
	}

	err = Pa_OpenDefaultStream(
		&_stream,
		0, // no input channels
		2, // stereo output
		paFloat32, // 32 bit floating point output
		SAMPLE_RATE,
		256, // frames per buffer
		AudioCallback, // callback function
		this);

	if (err != paNoError) {
		throw std::runtime_error(
			std::string("Failed to open stream: ") +
			Pa_GetErrorText(err));
	}

	err = Pa_StartStream(_stream);

	if (err != paNoError) {
		throw std::runtime_error(
			std::string("Failed to start stream: ") +
			Pa_GetErrorText(err));
	}
}

Audio::~Audio()
{
	PaError err = Pa_StopStream(_stream);

	if (err != paNoError) {
		Logger::Error() <<
			"Failed to stop stream: " <<
			Pa_GetErrorText(err);
	}

	err = Pa_CloseStream(_stream);

	if (err != paNoError) {
		Logger::Error() <<
			"Failed to close stream: " <<
			Pa_GetErrorText(err);
	}

	err = Pa_Terminate();

	if (err != paNoError) {
		Logger::Error() <<
			"Failed to terminate portaudio: " <<
			Pa_GetErrorText(err);
	}
}

void Audio::Submit(Buffer* buffer)
{
	_submitMutex.lock();

	_ringBuffer[_bufferEnd].buffer = buffer;
	_ringBuffer[_bufferEnd].position = 0;
	_ringBuffer[_bufferEnd].valid = true;

	buffer->Finished = false;

	if (_bufferEnd == RING_BUFFER_SIZE - 1) {
		_bufferEnd = 0;
	} else {
		++_bufferEnd;
	}

	_submitMutex.unlock();
}

int Audio::AudioCallback(
	const void* inputBuffer,
	void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)
{
	Audio* audio = static_cast<Audio*>(userData);
	float* out = static_cast<float*>(outputBuffer);

	memset(out, 0, sizeof(float) * framesPerBuffer * 2);

	size_t bufIdx = audio->_bufferStart;

	while (bufIdx != audio->_bufferEnd) {
		BufferData& buffer = audio->_ringBuffer[bufIdx];

		if (bufIdx == RING_BUFFER_SIZE - 1) {
			bufIdx = 0;
		} else {
			++bufIdx;
		}

		if (!buffer.valid) {
			continue;
		}

		if (!buffer.buffer->Active) {
			continue;
		}

		if (buffer.buffer->Discard) {
			buffer.buffer->Finished = true;
			buffer.valid = false;
			continue;
		}

		size_t inIdx = buffer.position;
		size_t inLen = buffer.buffer->Data.size();

		if (inIdx >= inLen) {
			continue;
		}

		for (size_t outIdx = 0; outIdx < framesPerBuffer * 2; ++outIdx)
		{
			out[outIdx] +=
				buffer.buffer->Data[inIdx] *
				buffer.buffer->Multiplier;

			++inIdx;

			if (inIdx >= inLen) {
				buffer.valid = false;
				buffer.buffer->Finished = true;
				break;
			}
		}

		buffer.position += framesPerBuffer * 2;
	}

	while (!audio->_ringBuffer[audio->_bufferStart].valid) {
		if (audio->_bufferStart == audio->_bufferEnd) {
			return 0;
		}

		if (audio->_bufferStart == RING_BUFFER_SIZE - 1) {
			audio->_bufferStart = 0;
		} else {
			++audio->_bufferStart;
		}
	}

	return 0;
}
