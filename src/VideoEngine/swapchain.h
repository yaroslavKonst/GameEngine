#ifndef _SWAPCHAIN_H
#define _SWAPCHAIN_H

#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdexcept>

#include "PhysicalDeviceSupport.h"
#include "CommandPool.h"
#include "MemorySystem.h"
#include "ImageHelper.h"
#include "pipeline.h"
#include "ModelDescriptor.h"
#include "SceneDescriptor.h"
#include "LightDescriptor.h"

class Swapchain
{
public:
	Swapchain(
		VkDevice device,
		VkSurfaceKHR surface,
		GLFWwindow* window,
		PhysicalDeviceSupport* deviceSupport,
		MemorySystem* memorySystem,
		VkSampleCountFlagBits msaaSamples,
		VkQueue graphicsQueue,
		VkQueue presentQueue,
		SceneDescriptor* scene,
		VkDescriptorSetLayout descriptorSetLayout,
		uint32_t maxLightCount,
		uint32_t shadowSize);

	~Swapchain();

	void Create();
	void Destroy();

	void MainLoop();
	void Stop();

	float GetScreenRatio()
	{
		return (float)_extent.width / (float)_extent.height;
	}

private:
	const uint32_t _maxFramesInFlight = 2;

	VkDevice _device;
	VkExtent2D _extent;
	VkSurfaceKHR _surface;
	GLFWwindow* _window;
	VkSampleCountFlagBits _msaaSamples;

	PhysicalDeviceSupport* _deviceSupport;
	MemorySystem* _memorySystem;

	SceneDescriptor* _scene;

	VkDescriptorSetLayout _descriptorSetLayout;

	VkQueue _graphicsQueue;
	VkQueue _presentQueue;

	bool _initialized;

	VkSurfaceFormatKHR ChooseSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR ChoosePresentMode(
		const std::vector<VkPresentModeKHR>& presentModes);

	VkExtent2D ChooseExtent(VkSurfaceCapabilitiesKHR capabilities);

	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _images;
	VkFormat _imageFormat;

	std::vector<BufferHelper::Buffer> _lightBuffers;
	VkDescriptorPool _lightDescriptorPool;
	VkDescriptorSetLayout _lightDescriptorSetLayout;
	std::vector<VkDescriptorSet> _lightDescriptorSets;
	std::vector<void*> _lightBufferMappings;
	uint32_t _maxLightCount;
	uint32_t _shadowSize;
	void CreateLightBuffers();
	void DestroyLightBuffers();

	VkFormat _hdrImageFormat;
	VkDescriptorPool _hdrDescriptorPool;
	VkDescriptorSetLayout _hdrDescriptorSetLayout;
	VkDescriptorSet _hdrDescriptorSet;
	std::vector<ImageHelper::Image> _hdrImages;
	std::vector<VkImageView> _hdrImageViews;
	VkSampler _hdrImageSampler;
	uint32_t _hdrImageCount;
	BufferHelper::Buffer _hdrBuffer;
	void CreateHDRResources();
	void DestroyHDRResources();

	ImageHelper::Image _colorImage;
	VkImageView _colorImageView;
	ImageHelper::Image _depthImage;
	VkImageView _depthImageView;
	std::vector<ImageHelper::Image> _shadowMapImages;
	std::vector<VkImageView> _shadowMapCubeImageViews;
	std::vector<VkImageView> _shadowMap2DImageViews;
	std::vector<VkSampler> _shadowMapSamplers;
	VkFormat _shadowFormat;
	void CreateRenderingImages();
	void DestroyRenderingImages();

	std::vector<VkImageView> _imageViews;
	void CreateImageViews();
	void DestroyImageViews();

	Pipeline* _pipeline;
	Pipeline* _rectanglePipeline;
	Pipeline* _skyboxPipeline;
	Pipeline* _shadowPipeline;
	Pipeline* _postprocessingPipeline;
	void CreatePipelines();
	void DestroyPipelines();

	CommandPool* _commandPool;
	CommandPool* _transferCommandPool;
	std::vector<VkCommandBuffer> _commandBuffers;

	void RecordCommandBuffer(
		VkCommandBuffer commandBuffer,
		uint32_t imageIndex,
		uint32_t currentFrame);

	bool _work;

	void DrawFrame();
	uint32_t _currentFrame;

	std::vector<VkSemaphore> _imageAvailableSemaphores;
	std::vector<VkSemaphore> _renderFinishedSemaphores;
	std::vector<VkFence> _inFlightFences;
	void CreateSyncObjects();
	void DestroySyncObjects();

	std::list<BufferHelper::Buffer> _stagingBuffers;
};

#endif
