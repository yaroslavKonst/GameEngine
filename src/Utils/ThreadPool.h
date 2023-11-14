#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <semaphore>
#include <mutex>
#include <functional>
#include <thread>
#include <list>
#include <set>

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
	std::mutex _queueMutex;
	std::counting_semaphore<> _queueSemaphore;

	std::counting_semaphore<> _resultSemaphore;
	std::set<uint32_t> _tasksInProgress;
	uint32_t _taskCount;
	uint32_t _lastId;

	bool _work;
	void ThreadFunction();

	void StartThreads(uint32_t threadCount);
};

#endif
