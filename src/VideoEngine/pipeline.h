#ifndef _PIPELINE_H
#define _PIPELINE_H

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

class Pipeline
{
public:
	Pipeline(
		VkDevice device,
		VkExtent2D extent,
		VkFormat colorAttachmentFormat);

	~Pipeline();

private:
	VkDevice _device;
	VkExtent2D _extent;
	VkFormat _colorAttachmentFormat;

	VkShaderModule CreateShaderModule(const uint8_t* data, size_t size);
	void DestroyShaderModule(VkShaderModule shaderModule);

	VkPipelineLayout _pipelineLayout;
	VkRenderPass _renderPass;
	void CreateRenderPass();
	void DestroyRenderPass();

	VkPipeline _pipeline;
};

#endif
