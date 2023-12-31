#include "CommandPool.h"

CommandPool::CommandPool(
	VkDevice device,
	uint32_t queueFamilyIndex,
	VkCommandPoolCreateFlags flags)
{
	_device = device;

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndex;
	poolInfo.flags = flags;

	VkResult res = vkCreateCommandPool(
		_device,
		&poolInfo,
		nullptr,
		&_commandPool);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool.");
	}
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(_device, _commandPool, nullptr);
}

VkCommandBuffer CommandPool::CreateCommandBuffer()
{
	VkCommandBuffer commandBuffer;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkResult res = vkAllocateCommandBuffers(
		_device,
		&allocInfo,
		&commandBuffer);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffer.");
	}

	return commandBuffer;
}

void CommandPool::DestroyCommandBuffer(VkCommandBuffer commandBuffer)
{
	vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

VkCommandBuffer CommandPool::StartOneTimeBuffer()
{
	VkCommandBuffer commandBuffer = CreateCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void CommandPool::EndOneTimeBuffer(
	VkCommandBuffer commandBuffer,
	VkQueue graphicsQueue)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	DestroyCommandBuffer(commandBuffer);
}
