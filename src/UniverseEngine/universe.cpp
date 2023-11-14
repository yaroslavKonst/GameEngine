#include "universe.h"

#include "../Logger/logger.h"

Universe::Universe(uint32_t tickDelayMS, Video* video)
{
	_video = video;
	_tickDelayMS = tickDelayMS;
	_threadPool = new ThreadPool();

	Logger::Verbose() << "Universe created.";
}

Universe::~Universe()
{
	delete _threadPool;
	_video = nullptr;
	Logger::Verbose() << "Universe destroyed.";
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

void Universe::RegisterCollisionEngine(CollisionEngine* engine)
{
	_collisionMutex.lock();
	_collisionEngines.insert(engine);
	_collisionMutex.unlock();
}

void Universe::RemoveCollisionEngine(CollisionEngine* engine)
{
	_collisionMutex.lock();
	_collisionEngines.erase(engine);
	_collisionMutex.unlock();
}

void Universe::MainLoop()
{
	_work = true;

	while (_work)
	{
		auto start = std::chrono::high_resolution_clock::now();

		_actorMutex.lock();
		std::set<Actor*> actors = _actors;
		_actorMutex.unlock();

		for (Actor* actor : actors) {
			_threadPool->Enqueue(
				[actor]() -> void {actor->TickEarly();});
		}

		_threadPool->WaitAll();

		_collisionMutex.lock();
		for (CollisionEngine* engine : _collisionEngines) {
			engine->Run(_threadPool);
		}

		_collisionMutex.unlock();

		for (Actor* actor : actors) {
			_threadPool->Enqueue(
				[actor]() -> void {actor->Tick();});
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

void Universe::Stop()
{
	_work = false;
}
