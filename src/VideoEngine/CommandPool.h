#ifndef _COMMAND_POOL_H
#define _COMMAND_POOL_H

#include <vulkan/vulkan.h>
#include <stdexcept>

class CommandPool
{
public:
	CommandPool(
		VkDevice device,
		uint32_t queueFamilyIndex,
		VkCommandPoolCreateFlags flags);
	~CommandPool();

	VkCommandBuffer CreateCommandBuffer();

private:
	VkDevice _device;
	VkCommandPool _commandPool;
};

#endif
