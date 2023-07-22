#include "square.h"

#include <stdexcept>
#include <cstring>
#include <cmath>

#include "../Utils/loader.h"

Square::Square(const char* texturePath, float depthMod)
{
	_depthMod = depthMod;

	int texWidth;
	int texHeight;

	std::vector<uint8_t> texData = Loader::LoadImage(
		texturePath,
		texWidth,
		texHeight);

	SetTexWidth(texWidth);
	SetTexHeight(texHeight);
	SetTexData(texData);

	SetDrawEnabled(true);

	SetRectanglePosition(glm::vec4(-1.0f, -1.0f, -0.1f, -0.1f));
	SetRectangleTexCoords(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	InputArea.x0 = -1;
	InputArea.y0 = -1;
	InputArea.x1 = -0.1;
	InputArea.y1 = -0.1;

	_time = 0;

	SetRectangleDepth(0);
	SetInputLayer(_depthMod);
	SetInputEnabled(true);
}

Square::~Square()
{
}

void Square::Tick()
{
	SetRectangleTexCoords(glm::vec4(
		0.0f,
		0.0f,
		1.0f + sinf(_time),
		1.0f + sinf(_time)));

	_time += 0.01;

	SetRectangleDepth(_depthMod * sinf(_time));

	if (_time >= 2 * M_PI) {
		_time = 0;
	}
}
