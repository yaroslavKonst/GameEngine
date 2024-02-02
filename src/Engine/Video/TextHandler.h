#ifndef _TEXT_HANDLER_H
#define _TEXT_HANDLER_H

#include <vector>

#include "video.h"
#include "../Utils/Text.h"

class TextHandler
{
	struct Glyph
	{
		Text::Glyph Data;
		bool HasTexture;
		uint32_t Texture;
	};

public:
	TextHandler(Video* video, const Text::GlyphCollection& collection);
	~TextHandler();

	Glyph GetGlyph(uint32_t code)
	{
		return _glyphs[code];
	}

	uint32_t GetMedianGlyphHeight()
	{
		return _medianHeight;
	}

private:
	std::map<uint32_t, Glyph> _glyphs;
	Video* _video;

	uint32_t _medianHeight;

	void LoadFont(const Text::GlyphCollection& collection);
};

#endif
