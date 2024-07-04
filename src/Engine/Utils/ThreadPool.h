#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <functional>
#include <thread>
#include <list>
#include <set>

#include "../Sync/mutex.h"
#include "../Sync/sem.h"

class ThreadPool
{
public:
	ThreadPool();
	ThreadPool(uint32_t threadCount);
	~ThreadPool();

	uint32_t Enqueue(std::function<void()> action, bool waitable = true);
	void Wait(uint32_t id);
	void WaitAll();

private:
	struct Task
	{
		std::function<void()> Action;
		bool Wait;
		uint32_t Id;
	};

	std::vector<std::thread*> _threads;
	std::list<Task> _queue;
	Sync::Mutex _queueMutex;
	Sync::Semaphore _queueSemaphore;

	Sync::Semaphore _resultSemaphore;
	std::set<uint32_t> _tasksInProgress;
	uint32_t _taskCount;
	uint32_t _lastId;

	volatile bool _work;
	void ThreadFunction();

	void StartThreads(uint32_t threadCount);
};

#endif
