#include "swapchain.h"

#include <algorithm>

#include "../Logger/logger.h"

Swapchain::Swapchain(
	VkDevice device,
	VkSurfaceKHR surface,
	GLFWwindow* window,
	PhysicalDeviceSupport* deviceSupport,
	MemorySystem* memorySystem,
	VkSampleCountFlagBits msaaSamples)
{
	_device = device;
	_surface = surface;
	_window = window;
	_deviceSupport = deviceSupport;
	_memorySystem = memorySystem;
	_msaaSamples = msaaSamples;

	Logger::Verbose("Swapchain constructor called.");

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
	Logger::Verbose("Create swapchain called.");

	if (_initialized) {
		Destroy();
	}

	PhysicalDeviceSupport::SwapchainSupportDetails supportDetails =
		_deviceSupport->QuerySwapchainSupport();
	PhysicalDeviceSupport::QueueFamilyIndices indices =
		_deviceSupport->FindQueueFamilies();

	VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(
		supportDetails.formats);
	VkPresentModeKHR presentMode = ChoosePresentMode(
		supportDetails.presentModes);

	_extent = ChooseExtent(supportDetails.capabilities);

	Logger::Verbose(
		std::string("Extent: ") + std::to_string(_extent.width) +
		"x" + std::to_string(_extent.height));


	uint32_t imageCount = supportDetails.capabilities.minImageCount + 7;

	if (supportDetails.capabilities.maxImageCount > 0) {
		imageCount = std::clamp(
			imageCount,
			supportDetails.capabilities.minImageCount,
			supportDetails.capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = _extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::vector<uint32_t> queueFamilyIndices = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount =
			static_cast<uint32_t>(queueFamilyIndices.size());
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = supportDetails.capabilities.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
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

	_imageFormat = surfaceFormat.format;

	CreateImages();
	CreateImageViews();
	CreatePipelines();

	_initialized = true;
}

void Swapchain::Destroy()
{
	DestroyPipelines();
	DestroyImageViews();
	DestroyImages();

	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	_initialized = false;
	Logger::Verbose("Swapchain destroyed.");
}

void Swapchain::CreateImages()
{
	_colorImage = ImageHelper::CreateImage(
		_device,
		_extent.width,
                _extent.height,
                1,
                VK_SAMPLE_COUNT_1_BIT, // _msaaSamples,
                _imageFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _memorySystem,
                _deviceSupport);

	VkFormat depthFormat = _deviceSupport->FindSupportedFormat(
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
		VK_FORMAT_FEATURE_TRANSFER_DST_BIT);

	_depthImage = ImageHelper::CreateImage(
		_device,
		_extent.width,
                _extent.height,
                1,
                VK_SAMPLE_COUNT_1_BIT, // _msaaSamples,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _memorySystem,
                _deviceSupport);
}

void Swapchain::DestroyImages()
{
	ImageHelper::DestroyImage(
		_device,
		_colorImage,
		_memorySystem);

	ImageHelper::DestroyImage(
		_device,
		_depthImage,
		_memorySystem);
}

void Swapchain::CreateImageViews()
{
	_imageViews.resize(_images.size());

	for (size_t i = 0; i < _images.size(); ++i) {
		_imageViews[i] = ImageHelper::CreateImageView(
			_device,
			_images[i],
			_imageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1);
	}
}

void Swapchain::DestroyImageViews()
{
	for (size_t i = 0; i < _images.size(); ++i) {
		ImageHelper::DestroyImageView(
			_device,
			_imageViews[i]);
	}
}

void Swapchain::CreatePipelines()
{
	_pipeline = new Pipeline(
		_device,
		_extent,
		_imageFormat);

	_pipeline->CreateFramebuffers(_imageViews);
}

void Swapchain::DestroyPipelines()
{
	_pipeline->DestroyFramebuffers();
	delete _pipeline;
}
