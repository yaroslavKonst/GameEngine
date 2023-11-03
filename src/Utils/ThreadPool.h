#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <semaphore>
#include <mutex>
#include <functional>
#include <thread>
#include <list>

class ThreadPool
{
public:
	ThreadPool();
	ThreadPool(uint32_t threadCount);
	~ThreadPool();

	void Enqueue(std::function<void()> action, bool wait = true);
	void Wait();

private:
	struct Task
	{
		std::function<void()> Action;
		bool Wait;
	};

	std::vector<std::thread*> _threads;
	std::list<Task> _queue;
	std::mutex _queueMutex;
	std::counting_semaphore<> _queueSemaphore;

	std::counting_semaphore<> _resultSemaphore;
	uint32_t _tasksInProgress;

	bool _work;
	void ThreadFunction();

	void StartThreads(uint32_t threadCount);
};

#endif
