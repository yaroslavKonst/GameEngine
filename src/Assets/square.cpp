#include "square.h"

#include <stdexcept>
#include <cstring>
#include <cmath>

#include "loader.h"

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

	SetActive(true);

	SetPosition(glm::vec4(-1.0f, -1.0f, -0.1f, -0.1f));
	SetTexCoords(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	Area.x0 = -1;
	Area.y0 = -1;
	Area.x1 = -0.1;
	Area.y1 = -0.1;

	_time = 0;

	SetDepth(0);
	SetLayer(_depthMod);
	SetMute(false);
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

	SetDepth(_depthMod * sinf(_time));

	if (_time >= 2 * M_PI) {
		_time = 0;
	}
}
