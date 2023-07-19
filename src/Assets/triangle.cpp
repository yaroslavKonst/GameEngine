#include "triangle.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"

Triangle::Triangle()
{
	SetModelVertices(
		{
			{-0.5f, -0.5f, 0.0f},
			{0.5f, -0.5f, 0.0f},
			{0.5f, 0.5f, 0.0f},
			{-0.5f, 0.5f, 0.0f},
			{-0.5f, -0.5f, -0.5f},
			{0.5f, -0.5f, -0.5f},
			{0.5f, 0.5f, -0.5f},
			{-0.5f, 0.5f, -0.5f}
		});

	SetTexCoords(
		{
			{1.0f, 0.0f},
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f},
			{1.0f, 0.0f},
			{0.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f}
		});

	SetModelIndices({0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4});

	SetModelActive(true);

	_angle = 0;


	int texWidth;
	int texHeight;
	int texChannels;

	stbi_uc* pixels = stbi_load(
		"../src/Assets/Resources/texture.jpg",
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
}

Triangle::~Triangle()
{
}

void Triangle::Tick()
{
	SetModelMatrix(glm::rotate(
		glm::mat4(1.0f),
		glm::radians((float)_angle),
		glm::vec3(0.0f, 0.0f, 1.0f)));

	_angle += 0.1;

	if (_angle >= 360) {
		_angle = 0;
	}
}
