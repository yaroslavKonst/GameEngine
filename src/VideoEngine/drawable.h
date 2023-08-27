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
	Drawable();
	virtual ~Drawable();

	virtual bool IsDrawEnabled()
	{
		return _active && _ready;
	}

	virtual void SetDrawEnabled(bool active)
	{
		_active = active;
	}

	virtual void _SetDrawReady(bool ready)
	{
		_ready = ready;
	}

	virtual bool DrawLight()
	{
		return _isLight;
	}

	virtual void SetDrawLight(bool value)
	{
		_isLight = value;
	}

	virtual const glm::vec4& GetColorMultiplier()
	{
		return _colorMultiplier;
	}

	virtual void SetColorMultiplier(const glm::vec4& value)
	{
		_colorMultiplier = value;
	}

private:
	bool _active;
	bool _ready;
	bool _isLight;
	glm::vec4 _colorMultiplier;
};

#endif
