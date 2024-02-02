#include "ThreadPool.h"

#include "../Logger/logger.h"

ThreadPool::ThreadPool() :
	_queueSemaphore(0),
	_resultSemaphore(0)
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
	_resultSemaphore(0)
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
	_taskCount = 0;
	_lastId = 0;

	for (size_t i = 0; i < _threads.size(); ++i) {
		_threads[i] = new std::thread(
			&ThreadPool::ThreadFunction,
			this);
	}

	Logger::Verbose() << "ThreadPool created. Threads: " << threadCount;
}

uint32_t ThreadPool::Enqueue(std::function<void()> action, bool waitable)
{
	Task task;
	task.Action = action;
	task.Wait = waitable;

	_queueMutex.lock();

	uint32_t id = 0;

	if (waitable) {
		++_lastId;

		while (_tasksInProgress.find(_lastId) !=
			_tasksInProgress.end())
		{
			++_lastId;
		}

		_tasksInProgress.insert(_lastId);
		++_taskCount;
		id = _lastId;
	}

	task.Id = id;
	_queue.push_front(task);
	_queueMutex.unlock();
	_queueSemaphore.release();

	return id;
}

void ThreadPool::Wait(uint32_t id)
{
	bool taskUnfinished = true;

	while (taskUnfinished) {
		_resultSemaphore.acquire();
		_queueMutex.lock();

		if (_tasksInProgress.find(id) == _tasksInProgress.end()) {
			taskUnfinished = false;
		}

		_queueMutex.unlock();
	}
}

void ThreadPool::WaitAll()
{
	while (_taskCount > 0) {
		_resultSemaphore.acquire();
	}
}

void ThreadPool::ThreadFunction()
{
	while (true) {
		_queueSemaphore.acquire();

		if (!_work) {
			break;
		}

		_queueMutex.lock();
		auto task = _queue.back();
		_queue.pop_back();
		_queueMutex.unlock();

		task.Action();

		if (task.Wait)
		{
			_queueMutex.lock();
			_tasksInProgress.erase(task.Id);
			--_taskCount;
			_queueMutex.unlock();

			_resultSemaphore.release();
		}
	}
}
