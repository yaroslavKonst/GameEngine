#ifndef _VK_INSTANCE_HANDER_H
#define _VK_INSTANCE_HANDER_H

#include <string>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace VkInstanceHandler
{
	void IncRef();
	void DecRef();

	VkInstance& GetInstance();

	void SetApplicationName(std::string name);
}

#endif
