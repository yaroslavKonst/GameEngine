#ifndef _RECTANGLE_H
#define _RECTANGLE_H

#include <vector>

#include "glm.h"
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
		bool ScaleX;
		bool ScaleY;
	};

	RectangleValues RectangleParams;

	Rectangle()
	{
		RectangleParams.TexCoords = {0, 0, 1, 1};
		RectangleParams.ScaleX = true;
		RectangleParams.ScaleY = false;
	}
};

#endif
