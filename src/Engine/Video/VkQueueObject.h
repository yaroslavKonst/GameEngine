#ifndef _VK_QUEUE_OBJECT_H
#define _VK_QUEUE_OBJECT_H

#include <vulkan/vulkan.h>

#include "../Sync/mutex.h"

struct VkQueueObject
{
	VkQueue Queue;
	Sync::Mutex Mutex;
};

#endif
