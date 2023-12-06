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
		std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
		VkSampleCountFlagBits MsaaSamples;
		const uint8_t* VertexShaderCode;
		size_t VertexShaderSize;
		const uint8_t* FragmentShaderCode;
		size_t FragmentShaderSize;
		const uint8_t* GeometryShaderCode;
		size_t GeometryShaderSize;
		std::vector<VkVertexInputBindingDescription>
			VertexBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription>
			VertexAttributeDescriptions;
		VkBool32 DepthTestEnabled;
		VkBool32 DepthWriteEnabled;
		uint32_t PushConstantRangeCount;
		VkPushConstantRange* PushConstants;
		bool ColorImage;
		bool DepthImage;
		VkImageLayout ColorImageFinalLayout;
		VkImageLayout DepthImageFinalLayout;
		bool ResolveImage;
		bool ClearColorImage;
		bool ClearDepthImage;
		bool InvertFace;
		bool AlphaBlending;
	};

	Pipeline(InitInfo* initInfo);

	~Pipeline();

	void CreateFramebuffers(
		const std::vector<VkImageView>& resolveImageViews,
		const std::vector<VkImageView>& colorImageViews,
		const std::vector<VkImageView>& depthImageViews,
		uint32_t layerCount = 1);
	void DestroyFramebuffers();

	void RecordCommandBuffer(
		VkCommandBuffer commandBuffer,
		uint32_t imageIndex,
		float colorClearAlphaValue = 1.0f);

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
	bool _color;
	bool _depth;
	bool _clearColorImage;

	VkShaderModule CreateShaderModule(const uint8_t* data, size_t size);
	void DestroyShaderModule(VkShaderModule shaderModule);

	VkPipelineLayout _pipelineLayout;
	VkRenderPass _renderPass;
	void CreateRenderPass(InitInfo* initInfo);
	void DestroyRenderPass();

	VkPipeline _pipeline;

	std::vector<VkFramebuffer> _framebuffers;
};

#endif
