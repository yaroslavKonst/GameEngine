#ifndef _VK_QUEUE_OBJECT_H
#define _VK_QUEUE_OBJECT_H

#include <mutex>
#include <vulkan/vulkan.h>

struct VkQueueObject
{
	VkQueue Queue;
	std::mutex Mutex;
};

#endif
