#ifndef _MODEL_DESCRIPTOR_H
#define _MODEL_DESCRIPTOR_H

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "MemorySystem.h"
#include "BufferHelper.h"

struct ModelDescriptor
{
	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;
	};

	uint32_t VertexCount;
	BufferHelper::Buffer VertexBuffer;

	uint32_t IndexCount;
	BufferHelper::Buffer IndexBuffer;

	static VkVertexInputBindingDescription GetVertexBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2>
		GetAttributeDescriptions();
};

#endif
