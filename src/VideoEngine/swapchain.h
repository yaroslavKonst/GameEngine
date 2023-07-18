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

class Swapchain
{
public:
	Swapchain(
		VkDevice device,
		VkSurfaceKHR surface,
		GLFWwindow* window,
		PhysicalDeviceSupport* deviceSupport,
		MemorySystem* memorySystem,
		VkSampleCountFlagBits msaaSamples);
	~Swapchain();

	void Create();
	void Destroy();

private:
	VkDevice _device;
	VkExtent2D _extent;
	VkSurfaceKHR _surface;
	GLFWwindow* _window;
	VkSampleCountFlagBits _msaaSamples;

	PhysicalDeviceSupport* _deviceSupport;
	MemorySystem* _memorySystem;

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
};

#endif
