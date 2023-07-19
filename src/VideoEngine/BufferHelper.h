#ifndef _BUFFER_HELPER_H
#define _BUFFER_HELPER_H

#include <vulkan/vulkan.h>

#include "MemorySystem.h"
#include "PhysicalDeviceSupport.h"

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
		PhysicalDeviceSupport* deviceSupport);

	void DestroyBuffer(
		VkDevice device,
		Buffer buffer,
		MemorySystem* memorySystem);
}

#endif
