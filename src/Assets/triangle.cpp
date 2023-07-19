#include "triangle.h"

Triangle::Triangle()
{
	SetModelVertices({{0.0f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}});
	SetModelColors(
		{
			{1.0f, 1.0f, 1.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f}
		});
	SetModelActive(true);
}
