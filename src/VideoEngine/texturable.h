#ifndef _TEXTURABLE_H
#define _TEXTURABLE_H

#include "drawable.h"

class Texturable : public Drawable
{
public:
	struct TexturableValues
	{
		uint32_t Diffuse;
		uint32_t Specular;
		bool IsLight;

		void SetAll(uint32_t texture)
		{
			Diffuse = texture;
			Specular = texture;
		}
	};

	TexturableValues TextureParams;

	Texturable()
	{
		TextureParams.IsLight = false;
	}
};

#endif
