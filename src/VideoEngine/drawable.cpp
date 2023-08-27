#include "drawable.h"

Drawable::Drawable()
{
	_active = false;
	_ready = false;
	_isLight = false;
	_colorMultiplier = glm::vec4(1.0f);
}

Drawable::~Drawable()
{
}
