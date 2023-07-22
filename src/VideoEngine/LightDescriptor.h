#ifndef _LIGHT_DESCRIPTOR_H
#define _LIGHT_DESCRIPTOR_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct LightDescriptor
{
	alignas(16) glm::vec3 Position;
	alignas(16) glm::vec3 Color;
	uint32_t Type;
};

#endif
