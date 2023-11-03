#ifndef _BUFFER_HELPER_H
#define _BUFFER_HELPER_H

#include <vulkan/vulkan.h>

#include "MemorySystem.h"
#include "PhysicalDeviceSupport.h"
#include "CommandPool.h"
#include "VkQueueObject.h"

namespace BufferHelper
{
	struct Buffer
	{
		VkBuffer Buffer;
		MemorySystem::Allocation Allocation;
	};

	Buffer CreateBuffer(
		VkDevice device,
		uint32_t size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		MemorySystem* memorySystem,
		PhysicalDeviceSupport* deviceSupport,
		bool mapped = false,
		uint32_t domain = 0);

	void DestroyBuffer(
		VkDevice device,
		Buffer buffer,
		MemorySystem* memorySystem,
		uint32_t domain = 0);

	void CopyBuffer(
		Buffer src,
		Buffer dst,
		CommandPool* commandPool,
		VkQueue graphicsQueue,
		VkCommandBuffer commandBufferExt = VK_NULL_HANDLE);

	void LoadDataToBuffer(
		VkDevice device,
		Buffer buffer,
		const void* data,
		size_t size,
		MemorySystem* memorySystem,
		PhysicalDeviceSupport* deviceSupport,
		CommandPool* commandPool,
		VkQueueObject* graphicsQueue,
		Buffer* stagingBufferPtr = nullptr,
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
}

#endif
