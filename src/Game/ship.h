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

	void TickEarly() override;
	void Tick() override;

	void Key(int key, int scancode, int action, int mods) override;
	bool Scroll(double xoffset, double yoffset) override;

private:
	Video* _video;
	CollisionEngine* _collisionEngine;

	BaseGrid* _baseGrid;
	MainGrid* _mainGrid;

	int32_t _buildX;
	int32_t _buildY;
	float _buildRotation;
	BaseBlock::Type _buildType;
	MainBlock::Type _mainBuildType;

	int32_t _buildLayer;
	int32_t _prevBuildLayer;

	glm::vec3 _position;
	glm::vec3 _speed;
	glm::vec3 _force;
	glm::mat4 _shipMatrix;

	void BaseLayer(int key, int scancode, int action, int mods);
	void MainLayer(int key, int scancode, int action, int mods);
};

#endif
