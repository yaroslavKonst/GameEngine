#include "universe.h"

#include "../Logger/logger.h"

Universe::Universe(uint32_t tickDelayMS)
{
	_tickDelayMS = tickDelayMS;
	_threadPool = new ThreadPool(3);

	Logger::Verbose("Universe created.");
}

Universe::~Universe()
{
	delete _threadPool;
	Logger::Verbose("Universe destroyed.");
}

void Universe::RegisterActor(Actor* actor)
{
	_actorMutex.lock();
	_actors.insert(actor);
	_actorMutex.unlock();
}

void Universe::RemoveActor(Actor* actor)
{
	_actorMutex.lock();
	_actors.erase(actor);
	_actorMutex.unlock();
}

void Universe::MainLoop()
{
	_work = true;

	while (_work)
	{
		_actorMutex.lock();
		auto start = std::chrono::high_resolution_clock::now();

		for (Actor* actor : _actors) {
			_threadPool->Enqueue(
				[actor]() -> void {actor->Tick();});
		}

		_threadPool->Wait();

		auto stop = std::chrono::high_resolution_clock::now();
		_actorMutex.unlock();

		uint32_t spentTimeMS =
			std::chrono::duration_cast<std::chrono::milliseconds>(
				stop - start).count();

		if (_tickDelayMS > spentTimeMS) {
			uint32_t timeToSleepMS = _tickDelayMS - spentTimeMS;

			Logger::Verbose(
				std::string("Sleeping ") +
				std::to_string(timeToSleepMS) + " ms");

			std::this_thread::sleep_for(
				std::chrono::milliseconds(timeToSleepMS));
		}
	}
}

void Universe::Stop()
{
	_work = false;
}
