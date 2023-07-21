#include "triangle.h"

#include <stdexcept>

#include "loader.h"

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

	SetModelTexCoords(
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

	SetDrawEnabled(true);

	_angle = 0;

	int texWidth;
	int texHeight;

	std::vector<uint8_t> texData = Loader::LoadImage(
		"../src/Assets/Resources/texture.jpg",
		texWidth,
		texHeight);

	SetTexWidth(texWidth);
	SetTexHeight(texHeight);
	SetTexData(texData);

	SetModelMatrix(glm::mat4(1.0f));
	SetModelInnerMatrix(glm::mat4(1.0f));

	SetModelInstances({
		glm::mat4(1.0f),
		glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, -0.2f)),
		glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.2f))
	});
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

	SetModelInnerMatrix(glm::rotate(
		glm::mat4(1.0f),
		glm::radians((float)_angle * 4),
		glm::vec3(0.0f, 0.0f, 1.0f)));

	_angle += 0.1;

	if (_angle >= 360) {
		_angle = 0;
	}
}
