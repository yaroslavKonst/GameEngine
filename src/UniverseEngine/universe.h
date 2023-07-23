#ifndef _UNIVERSE_H
#define _UNIVERSE_H

#include <set>
#include <chrono>
#include <mutex>
#include <thread>

#include "../Utils/ThreadPool.h"
#include "actor.h"
#include "../PhysicalEngine/CollisionEngine.h"

class Universe
{
public:
	Universe(uint32_t tickDelayMS);
	~Universe();

	void RegisterActor(Actor* actor);
	void RemoveActor(Actor* actor);

	void RegisterCollisionEngine(CollisionEngine* engine);
	void RemoveCollisionEngine(CollisionEngine* engine);

	void MainLoop();
	void Stop();

private:
	uint32_t _tickDelayMS;

	std::set<Actor*> _actors;
	std::mutex _actorMutex;

	std::set<CollisionEngine*> _collisionEngines;
	std::mutex _collisionMutex;

	bool _work;

	ThreadPool* _threadPool;
};

#endif
