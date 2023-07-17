#ifndef _SWAPCHAIN_H
#define _SWAPCHAIN_H

#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdexcept>

class Swapchain
{
public:
	Swapchain(
		VkDevice device,
		VkSurfaceKHR surface,
		GLFWwindow* window,
		VkSurfaceCapabilitiesKHR capabilities,
		std::vector<VkSurfaceFormatKHR> formats,
		std::vector<VkPresentModeKHR> presentModes,
		uint32_t graphicsQueueFamilyIndex,
		uint32_t presentQueueFamilyIndex);
	~Swapchain();

	void Create();
	void Destroy();

private:
	VkDevice _device;
	VkSurfaceCapabilitiesKHR _capabilities;
	VkSurfaceFormatKHR _surfaceFormat;
	VkPresentModeKHR _presentMode;
	VkExtent2D _extent;
	VkSurfaceKHR _surface;
	GLFWwindow* _window;
	uint32_t _graphicsQueueFamilyIndex;
	uint32_t _presentQueueFamilyIndex;

	bool _initialized;

	VkSurfaceFormatKHR ChooseSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR ChoosePresentMode(
		const std::vector<VkPresentModeKHR>& presentModes);

	VkExtent2D ChooseExtent(VkSurfaceCapabilitiesKHR capabilities);

	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _images;
	VkFormat _imageFormat;

	uint32_t _framesInFlight;
	uint32_t _currentFrame;
};

#endif
