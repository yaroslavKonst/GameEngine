#include "TextHandler.h"

#include <set>

#include "../Logger/logger.h"

TextHandler::TextHandler(TextureHandler* textureHandler)
{
	_textureHandler = textureHandler;
}

TextHandler::~TextHandler()
{
	for (auto& glyph : _glyphs) {
		if (glyph.second.HasTexture) {
			_textureHandler->RemoveTexture(glyph.second.Texture);
		}
	}
}

void TextHandler::LoadFont(const Text::GlyphCollection& collection)
{
	std::multiset<uint32_t> heights;

	for (auto& data : collection) {
		Glyph glyph{};
		glyph.Data = data.second;

		std::vector<uint8_t> buffer(glyph.Data.Bitmap.size() * 4);

		for (size_t i = 0; i < glyph.Data.Bitmap.size(); ++i) {
			buffer[i * 4] = glyph.Data.Bitmap[i];
			buffer[i * 4 + 1] = glyph.Data.Bitmap[i];
			buffer[i * 4 + 2] = glyph.Data.Bitmap[i];
			buffer[i * 4 + 3] = glyph.Data.Bitmap[i];
		}

		glyph.HasTexture = false;

		heights.insert(glyph.Data.Height);

		if (glyph.Data.Width > 0 && glyph.Data.Height > 0) {
			uint32_t texId = _textureHandler->AddTexture(
				glyph.Data.Width,
				glyph.Data.Height,
				buffer,
				false);

			glyph.Texture = texId;
			glyph.HasTexture = true;
		}

		glyph.Data.Bitmap.clear();

		_glyphs[data.first] = glyph;
	}

	auto it = heights.begin();

	for (size_t i = 0; i < heights.size() / 2; ++i) {
		++it;
	}

	_medianHeight = *it;
}
