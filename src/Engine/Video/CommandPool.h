#ifndef _COMMAND_POOL_H
#define _COMMAND_POOL_H

#include <vulkan/vulkan.h>
#include <stdexcept>

#include "VkQueueObject.h"

class CommandPool
{
public:
	CommandPool(
		VkDevice device,
		uint32_t queueFamilyIndex,
		VkCommandPoolCreateFlags flags);
	~CommandPool();

	VkCommandBuffer CreateCommandBuffer();
	void DestroyCommandBuffer(VkCommandBuffer commandBuffer);

	VkCommandBuffer StartOneTimeBuffer();
	void EndOneTimeBuffer(
		VkCommandBuffer commandBuffer,
		VkQueueObject* graphicsQueue);

private:
	VkDevice _device;
	VkCommandPool _commandPool;

	VkFence _fence;
};

#endif
