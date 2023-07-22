#ifndef _CAMERA_H
#define _CAMERA_H

#include <cmath>

#include "../UniverseEngine/actor.h"
#include "../VideoEngine/video.h"
#include "../Logger/logger.h"

class Camera : public Actor
{
public:
	Camera(Video* video)
	{
		_video = video;
		_angle = 0;
	}

	void Tick()
	{
		float angleInRad = glm::radians(_angle);
		_video->SetCameraPosition(glm::vec3(
			sinf(angleInRad) * 6,
			cosf(angleInRad) * 6,
			4));
		_video->SetCameraTarget(glm::vec3(0.0f, 0.0f, 0.0f));

		_angle += 0.1;

		if (_angle >= 360) {
			_angle = 0;
		}
	}

private:
	float _angle;
	Video* _video;
};

#endif
