#include "TimeEngine.h"

#include "../Logger/logger.h"

TimeEngine::TimeEngine(uint32_t tickDelayMS, Video* video)
{
	_video = video;
	_tickDelayMS = tickDelayMS;
	_threadPool = new ThreadPool();

	Logger::Verbose() << "Time engine created.";
}

TimeEngine::~TimeEngine()
{
	delete _threadPool;
	_video = nullptr;
	Logger::Verbose() << "Time engine destroyed.";
}

void TimeEngine::RegisterActor(Actor* actor)
{
	_actorMutex.lock();
	_actors.insert(actor);
	_actorMutex.unlock();
}

void TimeEngine::RemoveActor(Actor* actor)
{
	_actorMutex.lock();
	_actors.erase(actor);
	_actorMutex.unlock();
}

void TimeEngine::RegisterPhysicalEngine(PhysicalEngineBase* engine)
{
	_engineMutex.lock();
	_physicalEngines.insert(engine);
	_engineMutex.unlock();
}

void TimeEngine::RemovePhysicalEngine(PhysicalEngineBase* engine)
{
	_engineMutex.lock();
	_physicalEngines.erase(engine);
	_engineMutex.unlock();
}

void TimeEngine::MainLoop()
{
	_work = true;

	while (_work)
	{
		auto start = std::chrono::high_resolution_clock::now();

		double time = (double)_tickDelayMS / 1000.0;

		_actorMutex.lock();
		std::set<Actor*> actors = _actors;
		_actorMutex.unlock();

		for (Actor* actor : actors) {
			_threadPool->Enqueue(
				[actor, time]() -> void
				{
					actor->TickEarly(time);
				});
		}

		_threadPool->WaitAll();

		_engineMutex.lock();
		for (PhysicalEngineBase* engine : _physicalEngines) {
			engine->Run(_threadPool, time);
		}

		_engineMutex.unlock();

		for (Actor* actor : actors) {
			_threadPool->Enqueue(
				[actor, time]() -> void {actor->Tick(time);});
		}

		_threadPool->WaitAll();

		if (_video) {
			_video->SubmitScene();
		}

		auto stop = std::chrono::high_resolution_clock::now();

		uint32_t spentTimeMS =
			std::chrono::duration_cast<std::chrono::milliseconds>(
				stop - start).count();

		if (_tickDelayMS > spentTimeMS) {
			uint32_t timeToSleepMS = _tickDelayMS - spentTimeMS;

			if (timeToSleepMS < _tickDelayMS / 4) {
				Logger::Warning() << "Delay " <<
					_tickDelayMS <<
					". Sleeping " <<
					timeToSleepMS << " ms.";
			}

			std::this_thread::sleep_for(
				std::chrono::milliseconds(timeToSleepMS));
		} else {
			Logger::Warning() <<
				"Tick processing took tick delay.";
		}
	}
}

void TimeEngine::Stop()
{
	_work = false;
}
