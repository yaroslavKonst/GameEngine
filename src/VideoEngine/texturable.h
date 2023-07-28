#ifndef _TEXTURABLE_H
#define _TEXTURABLE_H

#include "drawable.h"

class Texturable : public Drawable
{
public:
	virtual uint32_t GetTexCount()
	{
		return _textures.size();
	}

	virtual void SetTexCount(uint32_t value)
	{
		_textures.resize(value);
	}

	virtual uint32_t GetTexture(uint32_t index)
	{
		return _textures[index];
	}

	virtual void SetTexture(uint32_t index, uint32_t value)
	{
		_textures[index] = value;
	}

	virtual const std::vector<uint32_t>& GetTextures()
	{
		return _textures;
	}

	virtual void SetTexture(const std::vector<uint32_t> textures)
	{
		_textures = textures;
	}

private:
	std::vector<uint32_t> _textures;
};

#endif
