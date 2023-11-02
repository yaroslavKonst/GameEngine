#ifndef _WORLD_H
#define _WORLD_H

#include "../UniverseEngine/universe.h"
#include "../VideoEngine/TextHandler.h"
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

	std::thread* _universeThread;

	TextHandler* _textHandler;
};

#endif
