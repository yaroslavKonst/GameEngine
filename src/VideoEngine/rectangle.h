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

class Rectangle : public Drawable
{
public:
	struct RectangleValues
	{
		uint32_t Texture;
		glm::vec4 Position;
		glm::vec4 TexCoords;
		float Depth;
	};

	RectangleValues RectangleParams;

	Rectangle()
	{
		RectangleParams.TexCoords = {0, 0, 1, 1};
	}
};

#endif
