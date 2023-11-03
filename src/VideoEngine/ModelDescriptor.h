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

#include "VkQueueObject.h"
#include "MemorySystem.h"
#include "BufferHelper.h"
#include "ImageHelper.h"
#include "model.h"
#include "../Utils/loader.h"

struct ModelDescriptor
{
	struct Vertex
	{
		alignas(16) glm::vec3 Pos;
		alignas(16) glm::vec3 Normal;
		alignas(8) glm::vec2 TexCoord;
	};

	uint32_t VertexCount;
	BufferHelper::Buffer VertexBuffer;

	uint32_t IndexCount;
	BufferHelper::Buffer IndexBuffer;

	uint32_t InstanceCount;
	BufferHelper::Buffer InstanceBuffer;

	int64_t MarkedFrameIndex;

	static std::vector<VkVertexInputBindingDescription>
		GetVertexBindingDescription();
	static std::vector<VkVertexInputAttributeDescription>
		GetAttributeDescriptions();

	static ModelDescriptor CreateModelDescriptor(
		const Loader::VertexData* model,
		VkDevice device,
		MemorySystem* memorySystem,
		PhysicalDeviceSupport* deviceSupport,
		VkQueueObject* graphicsQueue,
		CommandPool* commandPool);
	static void DestroyModelDescriptor(
		ModelDescriptor descriptor,
		VkDevice device,
		MemorySystem* memorySystem);
};

#endif
