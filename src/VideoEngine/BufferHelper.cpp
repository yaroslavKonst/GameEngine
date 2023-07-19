#include "BufferHelper.h"

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
			commandPool->CreateCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

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

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		commandPool->DestroyCommandBuffer(commandBuffer);
	}
}
