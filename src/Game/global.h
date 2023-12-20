#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "../UniverseEngine/universe.h"
#include "../PhysicalEngine/PhysicalEngine.h"
#include "../AudioEngine/audio.h"
#include "../VideoEngine/TextHandler.h"
#include "../Assets/localizer.h"

struct Common
{
	Video* video;
	Audio* audio;
	Universe* universe;
	PhysicalEngine* physicalEngine;
	TextHandler* textHandler;
	Localizer* localizer;
};

#endif
