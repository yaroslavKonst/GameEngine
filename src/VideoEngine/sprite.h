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
	struct SpriteValues
	{
		glm::vec3 Position;
		glm::vec3 Up;
		glm::vec2 Size;

		glm::vec4 TexCoords;
	};

	SpriteValues SpriteParams;

	Sprite()
	{
		SpriteParams.TexCoords = {0, 0, 1, 1};
	}
};

#endif
