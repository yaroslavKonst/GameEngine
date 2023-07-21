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
		_video->SetCameraDirection(glm::vec3(
			sinf(angleInRad),
			cosf(angleInRad),
			-0.7f));

		Logger::Verbose() << "Angle: " << _angle;

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
