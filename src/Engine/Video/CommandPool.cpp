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

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	res = vkCreateFence(_device, &fenceInfo, nullptr, &_fence);

	if (res != VK_SUCCESS) {
		vkDestroyCommandPool(_device, _commandPool, nullptr);

		throw std::runtime_error(
			"Failed to create command pool fence.");
	}
}

CommandPool::~CommandPool()
{
	vkDestroyFence(_device, _fence, nullptr);
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
	VkQueueObject* graphicsQueue)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkResetFences(_device, 1, &_fence);

	graphicsQueue->Mutex.Lock();
	vkQueueSubmit(graphicsQueue->Queue, 1, &submitInfo, _fence);
	graphicsQueue->Mutex.Unlock();

	vkWaitForFences(_device, 1, &_fence, VK_TRUE, UINT64_MAX);

	DestroyCommandBuffer(commandBuffer);
}
