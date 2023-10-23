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

	void Enqueue(std::function<void()> action);
	void Wait();

private:
	std::vector<std::thread*> _threads;
	std::list<std::function<void()>> _queue;
	std::mutex _queueMutex;
	std::counting_semaphore<> _queueSemaphore;

	std::counting_semaphore<> _resultSemaphore;
	std::counting_semaphore<> _threadReadySemaphore;
	uint32_t _tasksInProgress;

	bool _work;
	void ThreadFunction();

	void StartThreads(uint32_t threadCount);
};

#endif
