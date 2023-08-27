#ifndef _SPRITE_H
#define _SPRITE_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "texturable.h"

class Sprite : public Texturable
{
public:
	const glm::vec3& GetSpritePosition()
	{
		return _position;
	}

	void SetSpritePosition(const glm::vec3& value)
	{
		_position = value;
	}

	const glm::vec3& GetSpriteUp()
	{
		return _up;
	}

	void SetSpriteUp(const glm::vec3& value)
	{
		_up = value;
	}

	const glm::vec4& GetSpriteTexCoords()
	{
		return _texCoords;
	}

	void SetSpriteTexCoords(const glm::vec4& value)
	{
		_texCoords = value;
	}

	const glm::vec2& GetSpriteSize()
	{
		return _size;
	}

	void SetSpriteSize(const glm::vec2& value)
	{
		_size = value;
	}

private:
	glm::vec3 _position;
	glm::vec3 _up;
	glm::vec2 _size;

	glm::vec4 _texCoords;
};

#endif
