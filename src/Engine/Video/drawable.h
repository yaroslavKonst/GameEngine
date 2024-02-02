#ifndef _DRAWABLE_H
#define _DRAWABLE_H

#include "glm.h"

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
