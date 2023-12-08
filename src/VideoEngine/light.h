#ifndef _LIGHT_H
#define _LIGHT_H

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
		Spot = 1,
		Direct = 2
	};

	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec3 Direction;
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
