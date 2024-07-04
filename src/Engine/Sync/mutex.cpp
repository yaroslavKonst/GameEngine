#include "mutex.h"

#include <stdexcept>

#include "../Logger/logger.h"

Sync::Mutex::Mutex()
{
	pthread_mutex_init(&_mutex, nullptr);
}

Sync::Mutex::~Mutex()
{
	int err = pthread_mutex_destroy(&_mutex);

	if (err) {
		Logger::Error() << "Attempt to delete locked mutex.";
	}
}

void Sync::Mutex::Lock()
{
	int err = pthread_mutex_lock(&_mutex);

	if (err) {
		throw std::runtime_error("Failed to lock mutex.");
	}
}

void Sync::Mutex::Unlock()
{
	int err = pthread_mutex_unlock(&_mutex);

	if (err) {
		throw std::runtime_error("Failed to unlock mutex.");
	}
}
