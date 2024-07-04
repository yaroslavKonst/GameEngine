#include "audio.h"

#include <cstring>
#include <stdexcept>

#include "../Logger/logger.h"

#define RING_BUFFER_SIZE 1024 * 16

Audio::Audio() : _inBuffers(RING_BUFFER_SIZE), _outBuffers(RING_BUFFER_SIZE)
{
	_activeBuffers = nullptr;

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
		SampleRate,
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

	Logger::Verbose() << "Audio subsystem started.";
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

	while (!_inBuffers.IsEmpty()) {
		delete _inBuffers.Get();
	}

	while (!_outBuffers.IsEmpty()) {
		delete _outBuffers.Get();
	}

	while (_activeBuffers) {
		BufferData* tmp = _activeBuffers;
		_activeBuffers = _activeBuffers->next;
		delete tmp;
	}

	Logger::Verbose() << "Audio subsystem stopped.";
}

void Audio::Submit(Buffer* buffer)
{
	BufferData* bufferData = new BufferData();

	bufferData->buffer = buffer;
	buffer->Finished = false;

	_inBuffers.Insert(bufferData);

	_mutex.Lock();

	while (!_outBuffers.IsEmpty()) {
		BufferData* buffer = _outBuffers.Get();
		delete buffer;
	}

	_mutex.Unlock();
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

	while (!audio->_inBuffers.IsEmpty()) {
		BufferData* bd = audio->_inBuffers.Get();

		bd->next = audio->_activeBuffers;
		audio->_activeBuffers = bd;
	}

	BufferData** currentBuffer = &audio->_activeBuffers;

	while (*currentBuffer) {
		BufferData* buffer = *currentBuffer;

		if (buffer->buffer->Discard) {
			buffer->buffer->Finished = true;

			BufferData* deletedBuf = *currentBuffer;
			*currentBuffer = (*currentBuffer)->next;
			audio->_outBuffers.Insert(deletedBuf);

			continue;
		}

		if (!buffer->buffer->Active) {
			currentBuffer = &(*currentBuffer)->next;
			continue;
		}

		size_t inIdx = buffer->position;
		size_t inLen = buffer->buffer->Data.size();

		if (inIdx >= inLen) {
			buffer->buffer->Finished = true;

			BufferData* deletedBuf = *currentBuffer;
			*currentBuffer = (*currentBuffer)->next;
			audio->_outBuffers.Insert(deletedBuf);

			continue;
		}

		for (size_t outIdx = 0; outIdx < framesPerBuffer * 2; ++outIdx)
		{
			out[outIdx] +=
				buffer->buffer->Data[inIdx] *
				buffer->buffer->Multiplier;

			++inIdx;

			if (inIdx >= inLen) {
				break;
			}
		}

		buffer->position += framesPerBuffer * 2;

		currentBuffer = &(*currentBuffer)->next;
	}

	return 0;
}
