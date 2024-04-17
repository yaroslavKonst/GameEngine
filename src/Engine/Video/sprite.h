#ifndef _SPRITE_H
#define _SPRITE_H

#include "../Math/vec.h"
#include "texturable.h"

class Sprite : public Texturable
{
public:
	struct SpriteValues
	{
		Math::Vec<3> Position;
		Math::Vec<3> Up;
		Math::Vec<2> Size;
		double Offset;

		glm::vec4 TexCoords;
	};

	SpriteValues SpriteParams;

	Sprite()
	{
		SpriteParams.TexCoords = {0, 0, 1, 1};
		SpriteParams.Offset = 0;
	}
};

#endif
