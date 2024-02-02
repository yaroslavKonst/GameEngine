#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "../Engine/Time/TimeEngine.h"
#include "../Engine/Physics/PhysicalEngine.h"
#include "../Engine/Audio/audio.h"
#include "../Engine/Video/TextHandler.h"
#include "../Engine/Assets/localizer.h"

struct Common
{
	Video* video;
	Audio* audio;
	TimeEngine* universe;
	PhysicalEngine* physicalEngine;
	TextHandler* textHandler;
	Localizer* localizer;
};

#endif
