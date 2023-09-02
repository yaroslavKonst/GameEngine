#ifndef _SHIP_H
#define _SHIP_H

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/CollisionEngine.h"
#include "../UniverseEngine/actor.h"
#include "BaseGrid.h"
#include "MainGrid.h"

class Ship : public InputHandler, public Actor
{
public:
	Ship(Video* video, CollisionEngine* collisionEngine);
	~Ship();

	void Tick() override;

	void Key(int key, int scancode, int action, int mods) override;
	void BaseLayer(int key, int scancode, int action, int mods);
	void MainLayer(int key, int scancode, int action, int mods);

private:
	Video* _video;
	CollisionEngine* _collisionEngine;

	BaseGrid* _baseGrid;
	MainGrid* _mainGrid;

	int32_t _buildX;
	int32_t _buildY;
	BaseBlock::Type _buildType;
	MainBlock::Type _mainBuildType;

	int32_t _buildLayer;
	int32_t _prevBuildLayer;
};

#endif
