#ifndef _TIME_ENGINE_H
#define _TIME_ENGINE_H

#include <set>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>

#include "../Utils/ThreadPool.h"
#include "../Video/video.h"
#include "actor.h"
#include "../Physics/PhysicalEngineBase.h"

class TimeEngine
{
public:
	TimeEngine(uint32_t tickDelayMS, Video* video);
	~TimeEngine();

	void RegisterActor(Actor* actor);
	void RemoveActor(Actor* actor);

	void RegisterPhysicalEngine(PhysicalEngineBase* engine);
	void RemovePhysicalEngine(PhysicalEngineBase* engine);

	void MainLoop();
	void Stop();

private:
	uint32_t _tickDelayMS;

	Video* _video;

	std::set<Actor*> _actors;
	std::mutex _actorMutex;

	std::set<PhysicalEngineBase*> _physicalEngines;
	std::mutex _engineMutex;

	std::atomic<bool> _work;

	ThreadPool* _threadPool;
};

#endif
