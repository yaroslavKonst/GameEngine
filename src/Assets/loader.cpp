#include "loader.h"

#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"

namespace Loader
{
	std::vector<uint8_t> LoadImage(
		std::string file,
		int& width,
		int& height)
	{
		int channels;

		stbi_uc* pixels = stbi_load(
			file.c_str(),
			&width,
			&height,
			&channels,
			STBI_rgb_alpha);

		if (!pixels) {
			throw std::runtime_error(
				"Failed to load image from file");
		}

		std::vector<uint8_t> data(width * height * 4);
		memcpy(data.data(), pixels, data.size());

		stbi_image_free(pixels);

		return data;
	}
};
