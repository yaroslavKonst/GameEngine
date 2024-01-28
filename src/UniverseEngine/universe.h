#ifndef _UNIVERSE_H
#define _UNIVERSE_H

#include <set>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>

#include "../Utils/ThreadPool.h"
#include "../VideoEngine/video.h"
#include "actor.h"
#include "../PhysicalEngine/PhysicalEngineBase.h"

class Universe
{
public:
	Universe(uint32_t tickDelayMS, Video* video);
	~Universe();

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
