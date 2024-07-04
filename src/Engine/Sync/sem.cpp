#include "sem.h"

#include <stdexcept>

#include "../Logger/logger.h"

Sync::Semaphore::Semaphore(int value)
{
	int err = sem_init(&_sem, 0, value);

	if (err == -1) {
		throw std::runtime_error("Failed to create semaphore.");
	}
}

Sync::Semaphore::~Semaphore()
{
	int err = sem_destroy(&_sem);

	if (err == -1) {
		Logger::Error() << "Failed to destroy semaphore.";
	}
}

void Sync::Semaphore::Down()
{
	int err = sem_wait(&_sem);

	if (err == -1) {
		throw std::runtime_error("Failed to decrement semaphore.");
	}
}

void Sync::Semaphore::Up()
{
	int err = sem_post(&_sem);

	if (err == -1) {
		throw std::runtime_error("Failed to increment semaphore.");
	}
}
