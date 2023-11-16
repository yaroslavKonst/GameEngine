#ifndef _WORLD_H
#define _WORLD_H

#include "../UniverseEngine/universe.h"
#include "../AudioEngine/audio.h"
#include "../VideoEngine/TextHandler.h"
#include "ship.h"

class World : public Actor
{
public:
	World();
	~World();

	void Run();

	void Tick() override;

private:
	Video* _video;
	Audio* _audio;
	Universe* _universe;
	CollisionEngine* _collisionEngine;

	std::thread* _universeThread;

	TextHandler* _textHandler;
};

#endif
