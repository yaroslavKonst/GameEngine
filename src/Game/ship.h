#ifndef _SHIP_H
#define _SHIP_H

#include "../Engine/Video/video.h"
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
	bool MouseMoveRaw(double xoffset, double yoffset) override;

	void ActivateFlight(FlightControl* control)
	{
		_flightMode = true;
		_activeFlightControl = control;
	}

	void ActivateBuild()
	{
		_buildMode = true;
	}

private:
	Video* _video;
	CollisionEngine* _collisionEngine;

	BaseGrid* _baseGrid;
	MainGrid* _mainGrid;

	bool _flightMode;
	bool _buildMode;

	int32_t _buildX;
	int32_t _buildY;
	float _buildRotation;
	float _buildRotationStep;
	BaseBlock::Type _buildType;
	MainBlock::Type _mainBuildType;

	int32_t _buildLayer;
	int32_t _prevBuildLayer;

	glm::vec3 _position;
	glm::vec3 _rotation;
	glm::vec3 _linearSpeed;
	glm::vec3 _angularSpeed;
	glm::mat4 _shipMatrix;

	glm::vec3 _targetSpeed;
	glm::mat4 _rotationMatrix;
	bool _grounded;

	float _controlF;
	float _controlR;
	float _controlU;

	FlightControl* _activeFlightControl;

	void Build(int key, int scancode, int action, int mods);
	void BaseLayer(int key, int scancode, int action, int mods);
	void MainLayer(int key, int scancode, int action, int mods);

	void Flight(int key, int scancode, int action, int mods);
};

#endif
