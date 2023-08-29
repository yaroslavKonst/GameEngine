#include "Text.h"

#include <stdexcept>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "../Assets/package.h"

namespace Text
{
	GlyphCollection LoadFont(
		std::string name,
		const std::vector<uint32_t> codes)
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

		FT_Set_Pixel_Sizes(face, 0, 48);

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
}
