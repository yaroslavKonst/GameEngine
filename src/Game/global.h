#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "../UniverseEngine/universe.h"
#include "../PhysicalEngine/CollisionEngine.h"
#include "../AudioEngine/audio.h"
#include "../VideoEngine/TextHandler.h"
#include "../Assets/localizer.h"

struct Common
{
	Video* video;
	Audio* audio;
	Universe* universe;
	CollisionEngine* collisionEngine;
	TextHandler* textHandler;
	Localizer* localizer;
};

#endif
