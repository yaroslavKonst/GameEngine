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
	virtual ~Light()
	{
	}

	virtual const glm::vec3& GetLightPosition()
	{
		return _position;
	}

	virtual void SetLightPosition(const glm::vec3& value)
	{
		_position = value;
	}

	virtual const glm::vec3& GetLightColor()
	{
		return _color;
	}

	virtual void SetLightColor(const glm::vec3& value)
	{
		_color = value;
	}

private:
	glm::vec3 _position;
	glm::vec3 _color;
};

#endif
