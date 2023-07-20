#include "square.h"

#include <stdexcept>
#include <cstring>
#include <cmath>

//#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"

Square::Square()
{
	int texWidth;
	int texHeight;
	int texChannels;

	stbi_uc* pixels = stbi_load(
		"../src/Assets/Resources/transparent.png",
		&texWidth,
		&texHeight,
		&texChannels,
		STBI_rgb_alpha);

	if (!pixels) {
		throw std::runtime_error("Failed to load texture image.");
	}


	std::vector<uint8_t> texData(texWidth * texHeight * 4);
	memcpy(texData.data(), pixels, texData.size());

	SetTexWidth(texWidth);
	SetTexHeight(texHeight);
	SetTexData(texData);

	stbi_image_free(pixels);

	SetActive(true);

	SetPosition(glm::vec4(-1.0f, -1.0f, -0.1f, -0.1f));
	SetTexCoords(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	_time = 0;
}

Square::~Square()
{
}

void Square::Tick()
{
	SetTexCoords(glm::vec4(
		0.0f,
		0.0f,
		1.0f + sinf(_time),
		1.0f + sinf(_time)));

	_time += 0.01;

	if (_time >= 2 * M_PI) {
		_time = 0;
	}
}
