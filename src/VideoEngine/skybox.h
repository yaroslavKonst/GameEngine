#ifndef _SKYBOX_H
#define _SKYBOX_H

#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>

#include "ImageHelper.h"
#include "texturable.h"
#include "ModelDescriptor.h"

struct Skybox
{
	struct ShaderData
	{
		alignas(16) glm::vec3 Direction;
		alignas(16) glm::vec3 Up;
		alignas(16) glm::vec3 ColorModifier1;
		alignas(16) glm::vec3 ColorModifier2;
		alignas(16) glm::vec3 Gradient;
		alignas(4) float FOV;
		alignas(4) float Ratio;
		alignas(4) int32_t GradientEnabled;
		alignas(4) float GradientOffset;
	};
	
	uint32_t Texture[2];
	glm::vec3 ColorMultiplier[2];
	bool Enabled;

	bool GradientEnabled;
	float GradientOffset;
	glm::vec3 Gradient;

	Skybox()
	{
		Enabled = false;
		ColorMultiplier[0] = {1, 1, 1};
		ColorMultiplier[1] = {1, 1, 1};
		Texture[0] = 0;
		Texture[1] = 0;

		GradientEnabled = false;
		GradientOffset = 0;
		Gradient = {0, 0, 0};
	}
};

#endif
