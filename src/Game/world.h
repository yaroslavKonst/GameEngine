#ifndef _WORLD_H
#define _WORLD_H

#include "../UniverseEngine/universe.h"
#include "ship.h"

class World
{
public:
	World();
	~World();

	void Run();

private:
	Video* _video;
	Universe* _universe;
	CollisionEngine* _collisionEngine;

	std::mutex _sceneMutex;
	std::thread* _universeThread;

	uint32_t _shipBlockTexture;
};

#endif