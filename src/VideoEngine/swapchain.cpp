#include "swapchain.h"

#include <algorithm>
#include <iostream>

Swapchain::Swapchain(
	VkDevice device,
	VkSurfaceKHR surface,
	GLFWwindow* window,
	VkSurfaceCapabilitiesKHR capabilities,
	std::vector<VkSurfaceFormatKHR> formats,
	std::vector<VkPresentModeKHR> presentModes,
	uint32_t graphicsQueueFamilyIndex,
	uint32_t presentQueueFamilyIndex)
{
	_device = device;
	_surface = surface;
	_window = window;
	_capabilities = capabilities;
	_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
	_presentQueueFamilyIndex = presentQueueFamilyIndex;
	_surfaceFormat = ChooseSurfaceFormat(formats);
	_presentMode = ChoosePresentMode(presentModes);

	std::cout << "Swapchain created" << std::endl;

	_initialized = false;
}

Swapchain::~Swapchain()
{
	if (_initialized) {
		Destroy();
	}
}

VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& formats)
{
	for (const auto& format : formats) {
		bool accept =
			format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		if (accept) {
			return format;
		}
	}

	return formats[0];
}

VkPresentModeKHR Swapchain::ChoosePresentMode(
	const std::vector<VkPresentModeKHR>& presentModes)
{
	for (const auto& presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseExtent(VkSurfaceCapabilitiesKHR capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		int32_t width;
		int32_t height;

		glfwGetFramebufferSize(_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void Swapchain::Create()
{
	std::cout << "Create swapchain called" << std::endl;
	if (_initialized) {
		Destroy();
	}

	_extent = ChooseExtent(_capabilities);

	std::cout << "Extent: " << _extent.width << "x" <<
		_extent.height << std::endl;


	uint32_t imageCount = _capabilities.minImageCount + 7;

	if (_capabilities.maxImageCount > 0) {
		imageCount = std::clamp(
			imageCount,
			_capabilities.minImageCount,
			_capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = _surfaceFormat.format;
	createInfo.imageColorSpace = _surfaceFormat.colorSpace;
	createInfo.imageExtent = _extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::vector<uint32_t> queueFamilyIndices = {
		_graphicsQueueFamilyIndex,
		_presentQueueFamilyIndex
	};

	if (_graphicsQueueFamilyIndex != _presentQueueFamilyIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount =
			static_cast<uint32_t>(queueFamilyIndices.size());
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = _capabilities.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = _presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult res = vkCreateSwapchainKHR(
		_device,
		&createInfo,
		nullptr,
		&_swapchain);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain.");
	}

	vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
	_images.resize(imageCount);
	vkGetSwapchainImagesKHR(
		_device,
		_swapchain,
		&imageCount,
		_images.data());

	_imageFormat = _surfaceFormat.format;

	if (_images.size() > 3) {
		_framesInFlight = _images.size() - 3;
	} else {
		_framesInFlight = 2;
	}

	_currentFrame = 0;

	_initialized = true;
}

void Swapchain::Destroy()
{
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	_initialized = false;
	std::cout << "Swapchain destroyed" << std::endl;
}
