#ifndef _PIPELINE_H
#define _PIPELINE_H

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#include "ModelDescriptor.h"

class Pipeline
{
public:
	struct InitInfo
	{
		VkDevice Device;
		VkExtent2D Extent;
		VkFormat ColorAttachmentFormat;
		VkFormat DepthAttachmentFormat;
		VkDescriptorSetLayout DescriptorSetLayout;
		VkSampleCountFlagBits MsaaSamples;
		const uint8_t* VertexShaderCode;
		size_t VertexShaderSize;
		const uint8_t* FragmentShaderCode;
		size_t FragmentShaderSize;
		std::vector<VkVertexInputBindingDescription>
			VertexBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription>
			VertexAttributeDescriptions;
		VkBool32 DepthTestEnabled;
		uint32_t PushConstantRangeCount;
		VkPushConstantRange* PushConstants;
		bool ResolveImage;
		bool ClearInputBuffer;
	};

	Pipeline(InitInfo* initInfo);

	~Pipeline();

	void CreateFramebuffers(
		const std::vector<VkImageView>& imageViews,
		VkImageView colorImageView,
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
	VkSampleCountFlagBits _msaaSamples;
	bool _resolve;
	bool _clearInputBuffer;

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
