#ifndef _PIPELINE_H
#define _PIPELINE_H

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#include "ModelDescriptor.h"

class Pipeline
{
public:
	Pipeline(
		VkDevice device,
		VkExtent2D extent,
		VkFormat colorAttachmentFormat,
		VkFormat depthAttachmentFormat,
		VkDescriptorSetLayout descriptorSetLayout);

	~Pipeline();

	void CreateFramebuffers(
		const std::vector<VkImageView>& imageViews,
		VkImageView depthImageView);
	void DestroyFramebuffers();

	void RecordCommandBuffer(
		VkCommandBuffer commandBuffer,
		uint32_t imageIndex);

	VkPipelineLayout GetPipelineLayout()
	{
		return _pipelineLayout;
	}

private:
	VkDevice _device;
	VkExtent2D _extent;
	VkFormat _colorAttachmentFormat;
	VkFormat _depthAttachmentFormat;

	VkShaderModule CreateShaderModule(const uint8_t* data, size_t size);
	void DestroyShaderModule(VkShaderModule shaderModule);

	VkPipelineLayout _pipelineLayout;
	VkRenderPass _renderPass;
	void CreateRenderPass();
	void DestroyRenderPass();

	VkPipeline _pipeline;

	std::vector<VkFramebuffer> _framebuffers;
};

#endif
