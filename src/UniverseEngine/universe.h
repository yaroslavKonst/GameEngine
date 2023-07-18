#ifndef _UNIVERSE_H
#define _UNIVERSE_H

#include <set>
#include <chrono>
#include <mutex>
#include <thread>

#include "ThreadPool.h"

class Universe
{
public:
	class Actor
	{
	public:
		virtual ~Actor();
		virtual void Tick() = 0;
	};

	Universe(uint32_t tickDelayMS);
	~Universe();

	void RegisterActor(Actor* actor);
	void RemoveActor(Actor* actor);

	void MainLoop();
	void Stop();

private:
	uint32_t _tickDelayMS;

	std::set<Actor*> _actors;
	std::mutex _actorMutex;

	bool _work;

	ThreadPool* _threadPool;
};

#endif
