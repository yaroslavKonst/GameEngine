#ifndef _LIGHT_DESCRIPTOR_H
#define _LIGHT_DESCRIPTOR_H

#include "glm.h"

struct LightDescriptor
{
	alignas(16) glm::vec3 Position;
	alignas(16) glm::vec3 Color;
	alignas(16) glm::vec3 Direction;
	uint32_t Type;
	float Angle;
	float OuterAngle;
};

#endif
