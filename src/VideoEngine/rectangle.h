#ifndef _RECTANGLE_H
#define _RECTANGLE_H

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "texturable.h"

class Rectangle : public Texturable
{
public:
	Rectangle();
	virtual ~Rectangle();

	virtual glm::vec4 GetRectanglePosition()
	{
		return _position;
	}

	virtual void SetRectanglePosition(glm::vec4 position)
	{
		_position = position;
	}

	virtual glm::vec4 GetRectangleTexCoords()
	{
		return _texCoords;
	}

	virtual void SetRectangleTexCoords(glm::vec4 texCoords)
	{
		_texCoords = texCoords;
	}

	virtual float GetRectangleDepth()
	{
		return _depth;
	}

	virtual void SetRectangleDepth(float depth)
	{
		_depth = depth;
	}

private:
	glm::vec4 _position;
	glm::vec4 _texCoords;
	float _depth;
};

#endif
