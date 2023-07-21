#ifndef _SKYBOX_H
#define _SKYBOX_H

#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>

#include "ImageHelper.h"
#include "texturable.h"
#include "ModelDescriptor.h"

struct Skybox : public Texturable
{
	struct ShaderData
	{
		alignas(16) glm::vec3 Direction;
		alignas(16) glm::vec3 Up;
		alignas(4) float FOV;
		alignas(4) float Ratio;
	};
	
	ModelDescriptor Descriptor;
};

#endif
