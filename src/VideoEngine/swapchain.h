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
#include "model.h"
#include "ModelDescriptor.h"

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
		std::map<Model*, ModelDescriptor>* models);

	~Swapchain();

	void Create();
	void Destroy();

	void MainLoop();
	void Stop();

private:
	const uint32_t _maxFramesInFlight = 2;

	VkDevice _device;
	VkExtent2D _extent;
	VkSurfaceKHR _surface;
	GLFWwindow* _window;
	VkSampleCountFlagBits _msaaSamples;

	PhysicalDeviceSupport* _deviceSupport;
	MemorySystem* _memorySystem;

	std::map<Model*, ModelDescriptor>* _models;

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

	ImageHelper::Image _colorImage;
	ImageHelper::Image _depthImage;
	void CreateImages();
	void DestroyImages();

	std::vector<VkImageView> _imageViews;
	void CreateImageViews();
	void DestroyImageViews();

	Pipeline* _pipeline;
	void CreatePipelines();
	void DestroyPipelines();

	CommandPool* _commandPool;
	std::vector<VkCommandBuffer> _commandBuffers;

	void RecordCommandBuffer(
		VkCommandBuffer commandBuffer,
		uint32_t imageIndex);

	bool _work;

	void DrawFrame();
	uint32_t _currentFrame;

	std::vector<VkSemaphore> _imageAvailableSemaphores;
	std::vector<VkSemaphore> _renderFinishedSemaphores;
	std::vector<VkFence> _inFlightFences;
	void CreateSyncObjects();
	void DestroySyncObjects();
};

#endif
