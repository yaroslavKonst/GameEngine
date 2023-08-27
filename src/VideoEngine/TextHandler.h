#ifndef _TEXT_HANDLER_H
#define _TEXT_HANDLER_H

#include <vector>

#include "TextureHandler.h"
#include "../Utils/Text.h"

class TextHandler
{
	struct Glyph
	{
		Text::Glyph Data;
		uint32_t Texture;
	};

public:
	TextHandler(TextureHandler* textureHandler);
	~TextHandler();

	void LoadFont(const Text::GlyphCollection& collection);

private:
	std::map<uint32_t, Glyph> _glyphs;
	TextureHandler* _textureHandler;
};

#endif
