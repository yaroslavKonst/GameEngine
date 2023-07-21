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
#include "ImageHelper.h"

struct ModelDescriptor
{
	struct Vertex
	{
		alignas(16) glm::vec3 Pos;
		alignas(8) glm::vec2 TexCoord;
	};

	uint32_t VertexCount;
	BufferHelper::Buffer VertexBuffer;

	uint32_t IndexCount;
	BufferHelper::Buffer IndexBuffer;

	uint32_t InstanceCount;
	BufferHelper::Buffer InstanceBuffer;

	ImageHelper::Image TextureImage;
	VkImageView TextureImageView;
	VkSampler TextureSampler;

	VkDescriptorPool DescriptorPool;
	VkDescriptorSet DescriptorSet;

	static std::vector<VkVertexInputBindingDescription>
		GetVertexBindingDescription();
	static std::vector<VkVertexInputAttributeDescription>
		GetAttributeDescriptions();
};

#endif
