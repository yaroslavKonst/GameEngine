#include "Text.h"

#include <stdexcept>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "../Assets/package.h"

namespace Text
{
	GlyphCollection LoadFont(
		std::string name,
		const std::set<uint32_t>& codes)
	{
		FT_Library library;

		GlyphCollection collection;

		int error = FT_Init_FreeType(&library);

		if (error) {
			throw std::runtime_error(
				"Failed to init FreeType library.");
		}

		FT_Face face;

		auto fileData = Package::Instance()->GetData(name);

		error = FT_New_Memory_Face(
			library,
			fileData.data(),
			fileData.size(),
			0,
			&face);

		if (error) {
			throw std::runtime_error(
				"Failed to load font.");
		}

		FT_Set_Pixel_Sizes(face, 0, 100);

		for (uint32_t code : codes) {
			error = FT_Load_Char(face, code, FT_LOAD_RENDER);

			if (error) {
				throw std::runtime_error(
					"Failed to load glyph.");
			}

			Glyph glyph;
			glyph.Width = face->glyph->bitmap.width;
			glyph.Height = face->glyph->bitmap.rows;
			glyph.BearingX = face->glyph->bitmap_left;
			glyph.BearingY = face->glyph->bitmap_top;
			glyph.Advance = face->glyph->advance.x;
			glyph.Bitmap.resize(glyph.Width * glyph.Height);
			memcpy(
				glyph.Bitmap.data(),
				face->glyph->bitmap.buffer,
				glyph.Bitmap.size());

			collection[code] = glyph;
		}

		FT_Done_FreeType(library);

		return collection;
	}

	std::vector<uint32_t> DecodeUTF8(const std::string& text)
	{
		std::vector<uint32_t> res;

		size_t pos = 0;
		uint32_t charBytesRemaining = 0;
		uint32_t currChar;

		while (pos < text.size()) {
			uint8_t byte = text[pos];
			++pos;

			if (charBytesRemaining == 0) {
				currChar = 0;

				if (!(byte >> 7)) {
					currChar = byte;
				} else if ((byte & 0b11100000) == 0b11000000) {
					charBytesRemaining = 1;

					currChar |=
						(uint32_t)(byte & 0b00011111) <<
						(6 * charBytesRemaining);
				} else if ((byte & 0b11110000) == 0b11100000) {
					charBytesRemaining = 2;

					currChar |=
						(uint32_t)(byte & 0b00001111) <<
						(6 * charBytesRemaining);
				} else if ((byte & 0b11111000) == 0b11110000) {
					charBytesRemaining = 3;

					currChar |=
						(uint32_t)(byte & 0b00000111) <<
						(6 * charBytesRemaining);
				} else {
					throw std::runtime_error(
						"Invalid initial byte " +
						std::to_string(byte));
				}
			} else {
				--charBytesRemaining;

				if ((byte & 0b11000000) == 0b10000000) {
					currChar |=
						(uint32_t)(byte & 0b00111111) <<
						(6 * charBytesRemaining);
				} else {
					throw std::runtime_error(
						"Invalid additional byte " +
						std::to_string(byte));
				}
			}

			if (!charBytesRemaining) {
				res.push_back(currChar);
			}
		}

		if (charBytesRemaining > 0) {
			throw std::runtime_error("More bytes expected.");
		}

		return res;
	}
}
