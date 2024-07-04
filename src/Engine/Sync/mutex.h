#ifndef _MUTEX_H
#define _MUTEX_H

#include <pthread.h>

namespace Sync
{
	class Mutex
	{
	public:
		Mutex();
		~Mutex();

		Mutex(const Mutex& mutex) = delete;
		Mutex& operator=(const Mutex& mutex) = delete;

		void Lock();
		void Unlock();

	private:
		pthread_mutex_t _mutex;
	};
}

#endif
