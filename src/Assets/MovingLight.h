#ifndef _MOVING_LIGHT_H
#define _MOVING_LIGHT_H

#include "../UniverseEngine/actor.h"
#include "../VideoEngine/light.h"

class MovingLight : public Light, public Actor
{
public:
	MovingLight(float radius, float speed)
	{
		_radius = radius;
		_angle = 0;
		_speed = speed;
	}

	void Tick()
	{
		SetLightPosition(glm::vec3(
			sinf(_angle) * _radius,
			cosf(_angle) * _radius,
			3.0f));

		_angle += _speed;
	}

private:
	float _angle;
	float _radius;
	float _speed;
};


#endif
