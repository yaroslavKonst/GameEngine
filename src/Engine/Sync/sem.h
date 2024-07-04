#ifndef _SEM_H
#define _SEM_H

#include <semaphore.h>

namespace Sync
{
	class Semaphore
	{
	public:
		Semaphore(int value = 0);
		~Semaphore();

		Semaphore(const Semaphore& semaphore) = delete;
		Semaphore& operator=(const Semaphore& semaphore) = delete;

		void Down();
		void Up();

	private:
		sem_t _sem;
	};
}

#endif
