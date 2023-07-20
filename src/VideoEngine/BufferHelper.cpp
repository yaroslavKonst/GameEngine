#include "BufferHelper.h"

#include <cstring>

namespace BufferHelper
{
	Buffer CreateBuffer(VkDevice device,
		uint32_t size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		MemorySystem* memorySystem,
		PhysicalDeviceSupport* deviceSupport)
	{
		Buffer buffer;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult res = vkCreateBuffer(
			device,
			&bufferInfo,
			nullptr,
			&buffer.Buffer);
		
		if (res != VK_SUCCESS) {
			throw std::runtime_error(
				"Failed to create buffer.");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(
			device,
			buffer.Buffer,
			&memRequirements);

		MemorySystem::AllocationProperties allocProps;
		allocProps.MemoryTypeIndex = deviceSupport->FindMemoryType(
			memRequirements.memoryTypeBits,
			properties);
		allocProps.Alignment = memRequirements.alignment;

		buffer.Allocation = memorySystem->Allocate(
			memRequirements.size,
			allocProps);

		vkBindBufferMemory(
			device,
			buffer.Buffer,
			buffer.Allocation.Memory,
			buffer.Allocation.Offset);

		return buffer;
	}

	void DestroyBuffer(
		VkDevice device,
		Buffer buffer,
		MemorySystem* memorySystem)
	{
		vkDestroyBuffer(device, buffer.Buffer, nullptr);
		memorySystem->Free(buffer.Allocation);
	}

	void CopyBuffer(
		Buffer src,
		Buffer dst,
		CommandPool* commandPool,
		VkQueue graphicsQueue)
	{
		VkCommandBuffer commandBuffer =
			commandPool->StartOneTimeBuffer();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = src.Allocation.Size;

		vkCmdCopyBuffer(
			commandBuffer,
			src.Buffer,
			dst.Buffer,
			1,
			&copyRegion);

		commandPool->EndOneTimeBuffer(
			commandBuffer,
			graphicsQueue);
	}

	void LoadDataToBuffer(
		VkDevice device,
		Buffer buffer,
		const void* data,
		size_t size,
		MemorySystem* memorySystem,
		PhysicalDeviceSupport* deviceSupport,
		CommandPool* commandPool,
		VkQueue graphicsQueue)
	{
		Buffer stagingBuffer = CreateBuffer(
			device,
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			memorySystem,
			deviceSupport);

		void* mappedMemory;

		vkMapMemory(
			device,
			stagingBuffer.Allocation.Memory,
			stagingBuffer.Allocation.Offset,
			stagingBuffer.Allocation.Size,
			0,
			&mappedMemory);

		memcpy(mappedMemory, data, size);

		vkUnmapMemory(
			device,
			stagingBuffer.Allocation.Memory);

		CopyBuffer(
			stagingBuffer,
			buffer,
			commandPool,
			graphicsQueue);

		DestroyBuffer(
			device,
			stagingBuffer,
			memorySystem);
	}
}
