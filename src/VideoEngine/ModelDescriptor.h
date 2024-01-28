#ifndef _MODEL_DESCRIPTOR_H
#define _MODEL_DESCRIPTOR_H

#include <vulkan/vulkan.h>

#include "glm.h"
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
		alignas(8) glm::ivec2 MatrixIndex;
		alignas(8) glm::vec2 MatrixCoeff;
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
