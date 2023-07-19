#include "triangle.h"

Triangle::Triangle()
{
	SetModelVertices(
		{
			{-0.5f, -0.5f},
			{0.5f, -0.5f},
			{0.5f, 0.5f},
			{-0.5f, 0.5f}
		});

	SetModelColors(
		{
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 1.0f}
		});

	SetModelIndices({0, 1, 2, 2, 3, 0});

	SetModelActive(true);

	_angle = 0;
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
