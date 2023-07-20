#ifndef _TEXTURABLE_H
#define _TEXTURABLE_H

#include "drawable.h"

class Texturable : public Drawable
{
public:
	virtual uint32_t GetTexWidth()
	{
		return _texWidth;
	}

	virtual uint32_t GetTexHeight()
	{
		return _texHeight;
	}

	virtual const std::vector<uint8_t>& GetTexData()
	{
		return _texture;
	}

	virtual void SetTexWidth(uint32_t value)
	{
		_texWidth = value;
	}

	virtual void SetTexHeight(uint32_t value)
	{
		_texHeight = value;
	}

	virtual void SetTexData(const std::vector<uint8_t>& texture)
	{
		_texture = texture;
	}

private:
	uint32_t _texWidth;
	uint32_t _texHeight;
	std::vector<uint8_t> _texture;
};

#endif
