#ifndef _TEXT_H
#define _TEXT_H

#include <vector>
#include <map>
#include <cstdint>
#include <string>

namespace Text
{
	struct Glyph
	{
		std::vector<uint8_t> Bitmap;
		uint32_t Width;
		uint32_t Height;
		uint32_t BearingX;
		uint32_t BearingY;
		uint32_t Advance;
	};

	typedef std::map<uint32_t, Glyph> GlyphCollection;

	GlyphCollection LoadFont(
		std::string name,
		const std::vector<uint32_t> codes);
}

#endif
