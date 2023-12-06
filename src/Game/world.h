#ifndef _WORLD_H
#define _WORLD_H

#include "global.h"

class World : public Actor
{
public:
	World();
	~World();

	void Run();

	void Tick() override;

private:
	Common _common;

	std::thread* _universeThread;

	uint32_t _skyboxDay;
	uint32_t _skyboxNight;
	float _skyboxTime;
};

#endif
