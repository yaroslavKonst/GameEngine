#include "ThreadPool.h"

#include "../Logger/logger.h"

ThreadPool::ThreadPool() :
	_queueSemaphore(0),
	_resultSemaphore(0),
	_threadReadySemaphore(0)
{
	uint32_t threadCount = std::thread::hardware_concurrency();

	if (threadCount == 0) {
		threadCount = 2;

		Logger::Warning() <<
			"Failed to determine thread count. Default: " <<
			threadCount;
	} else if (threadCount > 4) {
		threadCount -= 2;
	}

	StartThreads(threadCount);
}

ThreadPool::ThreadPool(uint32_t threadCount) :
	_queueSemaphore(0),
	_resultSemaphore(0),
	_threadReadySemaphore(0)
{
	StartThreads(threadCount);
}

ThreadPool::~ThreadPool()
{
	_work = false;

	for (size_t i = 0; i < _threads.size(); ++i) {
		_queueSemaphore.release();
	}

	for (size_t i = 0; i < _threads.size(); ++i) {
		_threads[i]->join();
		delete _threads[i];
	}

	Logger::Verbose() << "ThreadPool stopped.";
}

void ThreadPool::StartThreads(uint32_t threadCount)
{
	_threads.resize(threadCount);
	_work = true;
	_tasksInProgress = 0;

	for (size_t i = 0; i < _threads.size(); ++i) {
		_threads[i] = new std::thread(
			&ThreadPool::ThreadFunction,
			this);
	}

	Logger::Verbose() << "ThreadPool created. Threads: " << threadCount;
}

void ThreadPool::Enqueue(std::function<void()> action)
{
	_threadReadySemaphore.acquire();
	_queueMutex.lock();
	_queue.push_front(action);
	++_tasksInProgress;
	_queueMutex.unlock();
	_queueSemaphore.release();
}

void ThreadPool::Wait()
{
	while (_tasksInProgress > 0) {
		_resultSemaphore.acquire();
		_queueMutex.lock();
		--_tasksInProgress;
		_queueMutex.unlock();
	}
}

void ThreadPool::ThreadFunction()
{
	while (true) {
		_threadReadySemaphore.release();
		_queueSemaphore.acquire();

		if (!_work) {
			break;
		}

		_queueMutex.lock();
		auto action = _queue.back();
		_queue.pop_back();
		_queueMutex.unlock();

		action();
		_resultSemaphore.release();
	}
}
