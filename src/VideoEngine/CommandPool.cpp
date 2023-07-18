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
