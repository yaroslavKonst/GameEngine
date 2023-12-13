#ifndef _DRAWABLE_H
#define _DRAWABLE_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

class Drawable
{
public:
	struct DrawableValues
	{
		bool Enabled;
		glm::vec4 ColorMultiplier;
	};

	DrawableValues DrawParams;

	Drawable()
	{
		DrawParams.Enabled = false;
		DrawParams.ColorMultiplier = glm::vec4(1.0f);
	}
};

#endif
