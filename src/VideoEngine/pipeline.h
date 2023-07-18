#ifndef _PIPELINE_H
#define _PIPELINE_H

#include <vulkan/vulkan.h>

class Pipeline
{
public:
	Pipeline(VkDevice device);
	~Pipeline();

private:
	VkDevice _device;
};

#endif
