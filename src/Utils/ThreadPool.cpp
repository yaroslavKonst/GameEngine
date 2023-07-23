#include "ThreadPool.h"

#include "../Logger/logger.h"

ThreadPool::ThreadPool(uint32_t threadCount) :
	_queueSemaphore(0),
	_resultSemaphore(0),
	_threadReadySemaphore(0)
{
	_threads.resize(threadCount);
	_work = true;
	_tasksInProgress = 0;

	for (size_t i = 0; i < _threads.size(); ++i) {
		_threads[i] = new std::thread(
			&ThreadPool::ThreadFunction,
			this);
		Logger::Verbose() << "ThreadPool thread created.";
	}
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
		Logger::Verbose() << "ThreadPool thread joined.";
	}
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
