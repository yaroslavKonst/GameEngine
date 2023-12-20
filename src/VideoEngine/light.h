#ifndef _LIGHT_H
#define _LIGHT_H

#include "../Math/vec.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

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
