#ifndef _LIGHT_H
#define _LIGHT_H

#include "../Math/vec.h"
#include "glm.h"

class Light
{
public:
	enum class Type
	{
		Point = 0,
		Spot = 1
	};

	Math::Vec<3> Position;
	Math::Vec<3> Direction;
	glm::vec3 Color;
	Type Type;
	float Angle;
	float AngleFade;

	bool Enabled;

	Light()
	{
		Enabled = false;
		Color = {1, 1, 1};
	}
};

#endif
