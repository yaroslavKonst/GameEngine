#include "swapchain.h"

#include <algorithm>
#include <cstring>

#include "SpriteDescriptor.h"

#include "../Logger/logger.h"

#include "shaders/spir-v/ObjectShader_vert.spv"
#include "shaders/spir-v/ObjectShader_frag.spv"
#include "shaders/spir-v/ObjectShaderDiscard_frag.spv"

#include "shaders/spir-v/RectangleShader_vert.spv"
#include "shaders/spir-v/RectangleShader_frag.spv"

#include "shaders/spir-v/SkyboxShader_vert.spv"
#include "shaders/spir-v/SkyboxShader_frag.spv"

#include "shaders/spir-v/ShadowShader_vert.spv"
#include "shaders/spir-v/ShadowShader_geom.spv"
#include "shaders/spir-v/ShadowShader_frag.spv"
#include "shaders/spir-v/ShadowShaderDiscard_vert.spv"
#include "shaders/spir-v/ShadowShaderDiscard_geom.spv"
#include "shaders/spir-v/ShadowShaderDiscard_frag.spv"

#include "shaders/spir-v/PostprocessingShader_vert.spv"
#include "shaders/spir-v/PostprocessingShader_frag.spv"

#include "shaders/spir-v/SpriteShader_vert.spv"
#include "shaders/spir-v/SpriteShader_frag.spv"

Swapchain::Swapchain(
	VkDevice device,
	VkSurfaceKHR surface,
	GLFWwindow* window,
	PhysicalDeviceSupport* deviceSupport,
	MemorySystem* memorySystem,
	VkSampleCountFlagBits msaaSamples,
	VkQueueObject* graphicsQueue,
	VkQueue presentQueue,
	SceneDescriptor* scene,
	VkDescriptorSetLayout descriptorSetLayout,
	uint32_t maxLightCount,
	uint32_t shadowSize)
{
	_device = device;
	_surface = surface;
	_window = window;
	_deviceSupport = deviceSupport;
	_memorySystem = memorySystem;
	_msaaSamples = msaaSamples;
	_graphicsQueue = graphicsQueue;
	_presentQueue = presentQueue;
	_scene = scene;
	_descriptorSetLayout = descriptorSetLayout;
	_maxLightCount = maxLightCount;
	_shadowSize = shadowSize;

	_hdrImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

	Logger::Verbose() << "Swapchain constructor called.";

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
	Logger::Verbose() << "Create swapchain called.";

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

	Logger::Verbose() <<
		"Extent: " << _extent.width << "x" << _extent.height;

	_transferCommandPool = new CommandPool(
		_device,
		indices.graphicsFamily.value(),
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

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

	CreateRenderingImages();
	CreateHDRResources();
	CreateImageViews();
	CreateLightBuffers();
	CreatePipelines();

	_commandPool = new CommandPool(
		_device,
		indices.graphicsFamily.value(),
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	_commandBuffers.resize(_maxFramesInFlight);

	for (size_t i = 0; i < _maxFramesInFlight; ++i) {
		_commandBuffers[i] = _commandPool->CreateCommandBuffer();
	}

	CreateSyncObjects();

	// TODO: Add rectangle screen ratio callback to input system.
	/*float ratio = GetScreenRatio();

	for (auto& rectangle : _scene->Rectangles) {
		rectangle->SetRectangleScreenRatio(ratio);
	}*/

	_currentFrame = 0;
	_initialized = true;
}

void Swapchain::Destroy()
{
	DestroySyncObjects();
	delete _commandPool;
	DestroyPipelines();
	DestroyLightBuffers();
	DestroyImageViews();
	DestroyHDRResources();
	DestroyRenderingImages();

	delete _transferCommandPool;

	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	_initialized = false;
	Logger::Verbose() << "Swapchain destroyed.";
}

void Swapchain::CreateRenderingImages()
{
	Logger::Verbose() << "Swapchain image format: " << _imageFormat;

	_colorImage = ImageHelper::CreateImage(
		_device,
		_extent.width,
                _extent.height,
                1,
                _msaaSamples,
                _hdrImageFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _memorySystem,
                _deviceSupport);

	_colorImageView = ImageHelper::CreateImageView(
		_device,
		_colorImage.Image,
		_hdrImageFormat,
		VK_IMAGE_ASPECT_COLOR_BIT,
		1);

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
                _msaaSamples,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _memorySystem,
                _deviceSupport);

	_depthImageView = ImageHelper::CreateImageView(
		_device,
		_depthImage.Image,
		depthFormat,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		1);

	ImageHelper::ChangeImageLayout(
		_depthImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		1,
		_transferCommandPool,
		_graphicsQueue);

	// Shadow maps.
	_shadowMapImages.resize(_maxLightCount);
	_shadowMapCubeImageViews.resize(_maxLightCount);
	_shadowMap2DImageViews.resize(_maxLightCount);
	_shadowMapSamplers.resize(_maxLightCount);

	uint32_t layerCount = 6;

	_shadowFormat = _deviceSupport->FindSupportedFormat(
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
		VK_FORMAT_FEATURE_TRANSFER_DST_BIT |
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

	Logger::Verbose() << "Shadow image format: " << _shadowFormat;

	for (uint32_t i = 0; i < _maxLightCount; ++i) {
		_shadowMapImages[i] = ImageHelper::CreateImage(
			_device,
			_shadowSize,
			_shadowSize,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			_shadowFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			_memorySystem,
			_deviceSupport,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			layerCount);

		_shadowMapCubeImageViews[i] = ImageHelper::CreateImageView(
			_device,
			_shadowMapImages[i].Image,
			_shadowFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			1,
			VK_IMAGE_VIEW_TYPE_CUBE,
			layerCount);

		_shadowMap2DImageViews[i] = ImageHelper::CreateImageView(
			_device,
			_shadowMapImages[i].Image,
			_shadowFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			1,
			VK_IMAGE_VIEW_TYPE_2D,
			layerCount);

		_shadowMapSamplers[i] = ImageHelper::CreateImageSampler(
			_device,
			_deviceSupport->GetPhysicalDevice(),
			1,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	}
}

void Swapchain::DestroyRenderingImages()
{
	ImageHelper::DestroyImageView(_device, _depthImageView);
	ImageHelper::DestroyImageView(_device, _colorImageView);

	ImageHelper::DestroyImage(
		_device,
		_colorImage,
		_memorySystem);

	ImageHelper::DestroyImage(
		_device,
		_depthImage,
		_memorySystem);

	for (uint32_t i = 0; i < _maxLightCount; ++i) {
		ImageHelper::DestroyImageSampler(
			_device,
			_shadowMapSamplers[i]);

		ImageHelper::DestroyImageView(
			_device,
			_shadowMapCubeImageViews[i]);

		ImageHelper::DestroyImageView(
			_device,
			_shadowMap2DImageViews[i]);

		ImageHelper::DestroyImage(
			_device,
			_shadowMapImages[i],
			_memorySystem);
	}
}

void Swapchain::CreateHDRResources()
{
	_hdrImageCount = 2;

	_hdrImages.resize(_hdrImageCount);
	_hdrImageViews.resize(_hdrImageCount);

	for (uint32_t i = 0; i < _hdrImageCount; ++i) {
		_hdrImages[i] = ImageHelper::CreateImage(
			_device,
			_extent.width,
			_extent.height,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			_hdrImageFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			_memorySystem,
			_deviceSupport);

		_hdrImageViews[i] = ImageHelper::CreateImageView(
			_device,
			_hdrImages[i].Image,
			_hdrImageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1);
	}

	_hdrImageSampler = ImageHelper::CreateImageSampler(
		_device,
		_deviceSupport->GetPhysicalDevice(),
		1,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

	uint32_t hdrBufferSize = sizeof(float) * 2;

	_hdrBuffer = BufferHelper::CreateBuffer(
		_device,
		hdrBufferSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_memorySystem,
		_deviceSupport);

	VkDescriptorSetLayoutBinding hdrSamplerLayoutBinding{};
	hdrSamplerLayoutBinding.binding = 0;
	hdrSamplerLayoutBinding.descriptorCount = _hdrImageCount;
	hdrSamplerLayoutBinding.descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	hdrSamplerLayoutBinding.pImmutableSamplers = nullptr;
	hdrSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding hdrBufferLayoutBinding{};
	hdrBufferLayoutBinding.binding = 1;
	hdrBufferLayoutBinding.descriptorCount = 1;
	hdrBufferLayoutBinding.descriptorType =
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	hdrBufferLayoutBinding.pImmutableSamplers = nullptr;
	hdrBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
		hdrSamplerLayoutBinding,
		hdrBufferLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult res = vkCreateDescriptorSetLayout(
		_device,
		&layoutInfo,
		nullptr,
		&_hdrDescriptorSetLayout);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to create HDR descriptor set layout.");
	}

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = _hdrImageCount;

	VkDescriptorPoolSize bufferPoolSize{};
	bufferPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferPoolSize.descriptorCount = 1;

	std::vector<VkDescriptorPoolSize> poolSizes = {
		poolSize,
		bufferPoolSize
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	res = vkCreateDescriptorPool(
		_device,
		&poolInfo,
		nullptr,
		&_hdrDescriptorPool);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to create HDR descriptor pool.");
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _hdrDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_hdrDescriptorSetLayout;

	res = vkAllocateDescriptorSets(
		_device,
		&allocInfo,
		&_hdrDescriptorSet);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to allocate HDR descriptor set.");
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites;

	std::vector<VkDescriptorImageInfo> imageInfos;

	for (uint32_t i = 0; i < _hdrImageCount; ++i) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout =
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = _hdrImageViews[i];
		imageInfo.sampler = _hdrImageSampler;

		imageInfos.push_back(imageInfo);
	}

	VkWriteDescriptorSet descriptorSamplerWrite{};
	descriptorSamplerWrite.sType =
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSamplerWrite.dstSet = _hdrDescriptorSet;
	descriptorSamplerWrite.dstBinding = 0;
	descriptorSamplerWrite.dstArrayElement = 0;

	descriptorSamplerWrite.descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSamplerWrite.descriptorCount = imageInfos.size();
	descriptorSamplerWrite.pImageInfo = imageInfos.data();

	descriptorWrites.push_back(descriptorSamplerWrite);

	memset(&descriptorSamplerWrite, 0, sizeof(VkWriteDescriptorSet));

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = _hdrBuffer.Buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = hdrBufferSize;

	descriptorSamplerWrite.sType =
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSamplerWrite.dstSet = _hdrDescriptorSet;
	descriptorSamplerWrite.dstBinding = 1;
	descriptorSamplerWrite.dstArrayElement = 0;

	descriptorSamplerWrite.descriptorType =
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSamplerWrite.descriptorCount = 1;
	descriptorSamplerWrite.pBufferInfo = &bufferInfo;

	descriptorWrites.push_back(descriptorSamplerWrite);

	vkUpdateDescriptorSets(
		_device,
		descriptorWrites.size(),
		descriptorWrites.data(),
		0,
		nullptr);
}

void Swapchain::DestroyHDRResources()
{
	vkDestroyDescriptorPool(_device, _hdrDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(
		_device,
		_hdrDescriptorSetLayout,
		nullptr);

	ImageHelper::DestroyImageSampler(
		_device,
		_hdrImageSampler);

	for (uint32_t i = 0; i < _hdrImageCount; ++i) {
		ImageHelper::DestroyImageView(_device, _hdrImageViews[i]);

		ImageHelper::DestroyImage(
			_device,
			_hdrImages[i],
			_memorySystem);
	}

	BufferHelper::DestroyBuffer(_device, _hdrBuffer, _memorySystem);
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
	Pipeline::InitInfo initInfo{};

	// Object pipeline
	initInfo.Device = _device;
	initInfo.Extent = _extent;
	initInfo.ColorAttachmentFormat = _hdrImageFormat;
	initInfo.DepthAttachmentFormat = _depthImage.Format;
	initInfo.DescriptorSetLayouts = {
		_descriptorSetLayout,
		_descriptorSetLayout,
		_lightDescriptorSetLayout
	};
	initInfo.MsaaSamples = _msaaSamples;
	initInfo.VertexShaderCode = ObjectShaderVert;
	initInfo.VertexShaderSize = sizeof(ObjectShaderVert);
	initInfo.FragmentShaderCode = ObjectShaderFrag;
	initInfo.FragmentShaderSize = sizeof(ObjectShaderFrag);
	initInfo.VertexBindingDescriptions =
		ModelDescriptor::GetVertexBindingDescription();
	initInfo.VertexAttributeDescriptions =
		ModelDescriptor::GetAttributeDescriptions();
	initInfo.DepthTestEnabled = VK_TRUE;
	initInfo.DepthWriteEnabled = VK_TRUE;
	initInfo.ResolveImage = false;
	initInfo.ClearColorImage = false;
	initInfo.ClearDepthImage = true;
	initInfo.ColorImage = true;
	initInfo.DepthImage = true;
	initInfo.InvertFace = false;
	initInfo.DepthImageFinalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	initInfo.ColorImageFinalLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkPushConstantRange pushConstants[2];
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(MVP);
	pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstants[1].offset = 192;
	pushConstants[1].size = sizeof(glm::vec4) * 2;
	initInfo.PushConstantRangeCount = 2;
	initInfo.PushConstants = pushConstants;

	_objectPipeline = new Pipeline(&initInfo);
	_objectPipeline->CreateFramebuffers(
		{_hdrImageViews[0]},
		{_colorImageView},
		{_depthImageView});

	initInfo.ClearDepthImage = false;
	initInfo.DepthWriteEnabled = VK_FALSE;

	_transparentObjectPipeline = new Pipeline(&initInfo);
	_transparentObjectPipeline->CreateFramebuffers(
		{_hdrImageViews[0]},
		{_colorImageView},
		{_depthImageView});

	initInfo.DepthWriteEnabled = VK_TRUE;
	initInfo.FragmentShaderCode = ObjectShaderDiscardFrag;
	initInfo.FragmentShaderSize = sizeof(ObjectShaderDiscardFrag);

	_holedObjectPipeline = new Pipeline(&initInfo);
	_holedObjectPipeline->CreateFramebuffers(
		{_hdrImageViews[0]},
		{_colorImageView},
		{_depthImageView});

	// Sprite pipeline
	initInfo.DepthTestEnabled = VK_TRUE;
	initInfo.DepthWriteEnabled = VK_FALSE;
	initInfo.ResolveImage = true;
	initInfo.VertexBindingDescriptions.clear();
	initInfo.VertexAttributeDescriptions.clear();
	initInfo.VertexShaderCode = SpriteShaderVert;
	initInfo.VertexShaderSize = sizeof(SpriteShaderVert);
	initInfo.FragmentShaderCode = SpriteShaderFrag;
	initInfo.FragmentShaderSize = sizeof(SpriteShaderFrag);
	initInfo.DescriptorSetLayouts = {
		_descriptorSetLayout,
		_descriptorSetLayout,
		_lightDescriptorSetLayout
	};
	initInfo.ClearColorImage = false;
	initInfo.ClearDepthImage = false;
	initInfo.ColorImageFinalLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	initInfo.PushConstantRangeCount = 2;
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(SpriteDescriptor);
	pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstants[1].offset = 192;
	pushConstants[1].size = sizeof(glm::vec4) * 2;

	_spritePipeline = new Pipeline(&initInfo);
	_spritePipeline->CreateFramebuffers(
		{_hdrImageViews[0]},
		{_colorImageView},
		{_depthImageView});

	// Rectangle pipeline
	initInfo.DepthTestEnabled = VK_FALSE;
	initInfo.DepthWriteEnabled = VK_TRUE;
	initInfo.ResolveImage = true;
	initInfo.VertexBindingDescriptions.clear();
	initInfo.VertexAttributeDescriptions.clear();
	initInfo.VertexShaderCode = RectangleShaderVert;
	initInfo.VertexShaderSize = sizeof(RectangleShaderVert);
	initInfo.FragmentShaderCode = RectangleShaderFrag;
	initInfo.FragmentShaderSize = sizeof(RectangleShaderFrag);
	initInfo.DescriptorSetLayouts = {
		_descriptorSetLayout
	};
	initInfo.ClearColorImage = true;
	initInfo.ClearDepthImage = true;
	initInfo.ColorImageFinalLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	initInfo.PushConstantRangeCount = 2;
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(glm::vec4) * 2;
	pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstants[1].offset = 32;
	pushConstants[1].size = sizeof(glm::vec4);

	_rectanglePipeline = new Pipeline(&initInfo);
	_rectanglePipeline->CreateFramebuffers(
		{_hdrImageViews[1]},
		{_colorImageView},
		{_depthImageView});

	// Skybox pipeline
	initInfo.DepthTestEnabled = VK_FALSE;
	initInfo.ResolveImage = false;
	initInfo.VertexBindingDescriptions.clear();
	initInfo.VertexAttributeDescriptions.clear();
	initInfo.VertexShaderCode = SkyboxShaderVert;
	initInfo.VertexShaderSize = sizeof(SkyboxShaderVert);
	initInfo.FragmentShaderCode = SkyboxShaderFrag;
	initInfo.FragmentShaderSize = sizeof(SkyboxShaderFrag);
	initInfo.DescriptorSetLayouts = {
		_descriptorSetLayout
	};
	initInfo.ClearColorImage = true;
	initInfo.ColorImageFinalLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	initInfo.PushConstantRangeCount = 1;
	pushConstants[0].stageFlags =
		VK_SHADER_STAGE_VERTEX_BIT |
		VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstants[0].size = sizeof(Skybox::ShaderData);

	_skyboxPipeline = new Pipeline(&initInfo);
	_skyboxPipeline->CreateFramebuffers(
		{_imageViews[0]},
		{_colorImageView},
		{_depthImageView});

	// Shadow pipeline
	initInfo.Extent.width = _shadowSize;
	initInfo.Extent.height = _shadowSize;
	initInfo.DepthAttachmentFormat = _shadowFormat;
	initInfo.DescriptorSetLayouts = {
		_lightDescriptorSetLayout
	};
	initInfo.MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.VertexShaderCode = ShadowShaderVert;
	initInfo.VertexShaderSize = sizeof(ShadowShaderVert);
	initInfo.FragmentShaderCode = ShadowShaderFrag;
	initInfo.FragmentShaderSize = sizeof(ShadowShaderFrag);
	initInfo.GeometryShaderCode = ShadowShaderGeom;
	initInfo.GeometryShaderSize = sizeof(ShadowShaderGeom);
	initInfo.VertexBindingDescriptions =
		ModelDescriptor::GetVertexBindingDescription();
	initInfo.VertexAttributeDescriptions =
		ModelDescriptor::GetAttributeDescriptions();
	initInfo.DepthTestEnabled = VK_TRUE;
	initInfo.DepthWriteEnabled = VK_TRUE;
	initInfo.ResolveImage = false;
	initInfo.ClearColorImage = false;
	initInfo.ClearDepthImage = true;
	initInfo.ColorImage = false;
	initInfo.DepthImage = true;
	initInfo.InvertFace = true;
	initInfo.DepthImageFinalLayout =
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	initInfo.PushConstantRangeCount = 2;
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(MVP);
	pushConstants[1].stageFlags =
		VK_SHADER_STAGE_GEOMETRY_BIT |
		VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstants[1].offset = 192;
	pushConstants[1].size = sizeof(glm::vec3) + sizeof(float) * 2;

	_shadowPipeline = new Pipeline(&initInfo);
	_shadowPipeline->CreateFramebuffers(
		{_imageViews[0]},
		{_colorImageView},
		_shadowMap2DImageViews,
		6);

	initInfo.DescriptorSetLayouts = {
		_lightDescriptorSetLayout,
		_descriptorSetLayout
	};

	initInfo.VertexShaderCode = ShadowShaderDiscardVert;
	initInfo.VertexShaderSize = sizeof(ShadowShaderDiscardVert);
	initInfo.FragmentShaderCode = ShadowShaderDiscardFrag;
	initInfo.FragmentShaderSize = sizeof(ShadowShaderDiscardFrag);
	initInfo.GeometryShaderCode = ShadowShaderDiscardGeom;
	initInfo.GeometryShaderSize = sizeof(ShadowShaderDiscardGeom);
	initInfo.ClearDepthImage = false;

	_shadowDiscardPipeline = new Pipeline(&initInfo);
	_shadowDiscardPipeline->CreateFramebuffers(
		{_imageViews[0]},
		{_colorImageView},
		_shadowMap2DImageViews,
		6);

	// Postprocessing pipeline.
	initInfo.Device = _device;
	initInfo.Extent = _extent;
	initInfo.ColorAttachmentFormat = _imageFormat;
	initInfo.DepthAttachmentFormat = _depthImage.Format;
	initInfo.DescriptorSetLayouts = {
		_hdrDescriptorSetLayout
	};
	initInfo.MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.VertexShaderCode = PostprocessingShaderVert;
	initInfo.VertexShaderSize = sizeof(PostprocessingShaderVert);
	initInfo.FragmentShaderCode = PostprocessingShaderFrag;
	initInfo.FragmentShaderSize = sizeof(PostprocessingShaderFrag);
	initInfo.GeometryShaderSize = 0;
	initInfo.VertexBindingDescriptions.clear();
	initInfo.VertexAttributeDescriptions.clear();
	initInfo.DepthTestEnabled = VK_FALSE;
	initInfo.ResolveImage = false;
	initInfo.ClearColorImage = true;
	initInfo.ColorImage = true;
	initInfo.DepthImage = false;
	initInfo.InvertFace = false;
	initInfo.DepthImageFinalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	initInfo.ColorImageFinalLayout =
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	pushConstants[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstants[0].offset = 0;
	pushConstants[0].size = sizeof(MVP);
	pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstants[1].offset = 192;
	pushConstants[1].size = sizeof(glm::vec3) + sizeof(uint32_t);
	initInfo.PushConstantRangeCount = 1;
	initInfo.PushConstants = pushConstants;

	_postprocessingPipeline = new Pipeline(&initInfo);
	_postprocessingPipeline->CreateFramebuffers(
		{_colorImageView},
		_imageViews,
		{_depthImageView});
}

void Swapchain::DestroyPipelines()
{
	_spritePipeline->DestroyFramebuffers();
	delete _spritePipeline;

	_postprocessingPipeline->DestroyFramebuffers();
	delete _postprocessingPipeline;

	_shadowPipeline->DestroyFramebuffers();
	delete _shadowPipeline;
	_shadowDiscardPipeline->DestroyFramebuffers();
	delete _shadowDiscardPipeline;

	_skyboxPipeline->DestroyFramebuffers();
	delete _skyboxPipeline;

	_rectanglePipeline->DestroyFramebuffers();
	delete _rectanglePipeline;

	_objectPipeline->DestroyFramebuffers();
	delete _objectPipeline;

	_transparentObjectPipeline->DestroyFramebuffers();
	delete _transparentObjectPipeline;

	_holedObjectPipeline->DestroyFramebuffers();
	delete _holedObjectPipeline;
}

void Swapchain::CreateLightBuffers()
{
	VkDescriptorSetLayoutBinding lightBufferLayoutBinding{};
	lightBufferLayoutBinding.binding = 0;
	lightBufferLayoutBinding.descriptorCount = 1;
	lightBufferLayoutBinding.descriptorType =
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightBufferLayoutBinding.pImmutableSamplers = nullptr;
	lightBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT |
		VK_SHADER_STAGE_GEOMETRY_BIT;

	VkDescriptorSetLayoutBinding lightSamplerLayoutBinding{};
	lightSamplerLayoutBinding.binding = 1;
	lightSamplerLayoutBinding.descriptorCount = _maxLightCount;
	lightSamplerLayoutBinding.descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	lightSamplerLayoutBinding.pImmutableSamplers = nullptr;
	lightSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
		lightBufferLayoutBinding,
		lightSamplerLayoutBinding,
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult res = vkCreateDescriptorSetLayout(
		_device,
		&layoutInfo,
		nullptr,
		&_lightDescriptorSetLayout);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to create light descriptor set layout.");
	}

	_lightBuffers.resize(_maxFramesInFlight);
	_lightDescriptorSets.resize(_maxFramesInFlight);

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = _maxFramesInFlight;

	VkDescriptorPoolSize samplerPoolSize{};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = _maxFramesInFlight * _maxLightCount;

	std::vector<VkDescriptorPoolSize> poolSizes = {
		poolSize,
		samplerPoolSize
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = _maxFramesInFlight;

	res = vkCreateDescriptorPool(
		_device,
		&poolInfo,
		nullptr,
		&_lightDescriptorPool);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to create light descriptor pool.");
	}

	std::vector<VkDescriptorSetLayout> layouts(
		_maxFramesInFlight,
		_lightDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _lightDescriptorPool;
	allocInfo.descriptorSetCount = _maxFramesInFlight;
	allocInfo.pSetLayouts = layouts.data();

	res = vkAllocateDescriptorSets(
		_device,
		&allocInfo,
		_lightDescriptorSets.data());

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to allocate light descriptor sets.");
	}

	//uint32_t bufferSize = sizeof(LightDescriptor) * _maxLightCount + 16 +
	//	sizeof(glm::mat4) * 6 * _maxLightCount;
	uint32_t bufferSize = 1024 + sizeof(glm::mat4) * 6 * _maxLightCount;

	for (size_t i = 0; i < _maxFramesInFlight; ++i) {
		_lightBuffers[i] = BufferHelper::CreateBuffer(
			_device,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			_memorySystem,
			_deviceSupport,
			true);
	}

	for (size_t i = 0; i < _maxFramesInFlight; ++i) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = _lightBuffers[i].Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = bufferSize;

		std::vector<VkWriteDescriptorSet> descriptorWrites;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = _lightDescriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType =
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		descriptorWrites.push_back(descriptorWrite);

		std::vector<VkDescriptorImageInfo> imageInfos;

		for (size_t l = 0; l < _maxLightCount; ++l) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout =
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = _shadowMapCubeImageViews[l];
			imageInfo.sampler = _shadowMapSamplers[l];

			imageInfos.push_back(imageInfo);
		}

		VkWriteDescriptorSet descriptorSamplerWrite{};
		descriptorSamplerWrite.sType =
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorSamplerWrite.dstSet = _lightDescriptorSets[i];
		descriptorSamplerWrite.dstBinding = 1;
		descriptorSamplerWrite.dstArrayElement = 0;

		descriptorSamplerWrite.descriptorType =
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSamplerWrite.descriptorCount = imageInfos.size();
		descriptorSamplerWrite.pImageInfo = imageInfos.data();

		descriptorWrites.push_back(descriptorSamplerWrite);

		vkUpdateDescriptorSets(
			_device,
			descriptorWrites.size(),
			descriptorWrites.data(),
			0,
			nullptr);
	}
}

void Swapchain::DestroyLightBuffers()
{
	vkDestroyDescriptorPool(_device, _lightDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(
		_device,
		_lightDescriptorSetLayout,
		nullptr);

	for (size_t i = 0; i < _maxFramesInFlight; ++i) {
		BufferHelper::DestroyBuffer(
			_device,
			_lightBuffers[i],
			_memorySystem);
	}
}

void Swapchain::RecordCommandBuffer(
	VkCommandBuffer commandBuffer,
	uint32_t imageIndex,
	uint32_t currentFrame)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	VkResult res = vkBeginCommandBuffer(
		commandBuffer,
		&beginInfo);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to begin recording command buffer.");
	}

	_scene->LoadToDrawn();

	// MVP.
	MVP mvp;
	glm::mat4 view = glm::lookAt(
		_scene->DrawnScene.CameraPosition,
		_scene->DrawnScene.CameraPosition +
		_scene->DrawnScene.CameraDirection,
		_scene->DrawnScene.CameraUp);
	mvp.ProjView = glm::perspective(
		glm::radians((float)_scene->DrawnScene.FOV),
		(float)_extent.width / (float)_extent.height,
		0.01f,
		1000.0f) * view;

	// ViewPort and scissor.
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_extent.width);
	viewport.height = static_cast<float>(_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = _extent;

	// Skybox pipeline.
	_skyboxPipeline->RecordCommandBuffer(commandBuffer, 0);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	bool validSkybox =
		_scene->skybox._IsDrawEnabled() &&
		_scene->Textures->CheckTexture(_scene->skybox.Texture);

	if (validSkybox) {
		Skybox::ShaderData shaderData;
		shaderData.Direction = _scene->DrawnScene.CameraDirection;
		shaderData.Up = _scene->DrawnScene.CameraUp;
		shaderData.FOV = glm::radians((float)_scene->DrawnScene.FOV);
		shaderData.Ratio = (float)_extent.width / (float)_extent.height;
		shaderData.ColorModifier = _scene->skybox.GetColorMultiplier();

		vkCmdPushConstants(
			commandBuffer,
			_skyboxPipeline->GetPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT |
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(Skybox::ShaderData),
			&shaderData);

		auto& tex = _scene->Textures->GetTexture(
			_scene->skybox.Texture);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			_skyboxPipeline->GetPipelineLayout(),
			0,
			1,
			&tex.DescriptorSet,
			0,
			nullptr);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);

	// Object pipeline.
	// Instance buffers update.
	for (auto& buffer : _stagingBuffers) {
		BufferHelper::DestroyBuffer(
			_device,
			buffer,
			_memorySystem);
	}

	_stagingBuffers.clear();

	/*for (auto& model : _scene->Models) {
		if (!model->_IsDrawEnabled()) {
			continue;
		}

		// TODO
		if (!model.first->_GetModelInstancesUpdated()) {
			BufferHelper::DestroyBuffer(
				_device,
				model.second.InstanceBuffer,
				_memorySystem);

			auto& instances = model.first->GetModelInstances();

			model.second.InstanceBuffer =
				BufferHelper::CreateBuffer(
					_device,
					instances.size() * sizeof(glm::mat4),
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
					VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					_memorySystem,
					_deviceSupport);

			BufferHelper::Buffer stagingBuffer;

			BufferHelper::LoadDataToBuffer(
				_device,
				model.second.InstanceBuffer,
				instances.data(),
				instances.size() * sizeof(glm::mat4),
				_memorySystem,
				_deviceSupport,
				nullptr,
				VK_NULL_HANDLE,
				&stagingBuffer,
				commandBuffer);

			model.second.InstanceCount = instances.size();

			_stagingBuffers.push_back(stagingBuffer);
			model.first->_SetModelInstancesUpdated();
		}
	}*/

	// Light transforms.
	std::multimap<float, Light*> orderedLights;

	for (auto& light : _scene->DrawnScene.Lights) {
		orderedLights.insert({glm::length(
			light.GetLightPosition() -
				_scene->DrawnScene.CameraPosition),
			&light
		});
	}

	uint32_t selectedLights = 0;

	LightDescriptor* lightDescriptors = reinterpret_cast<LightDescriptor*>(
		(char*)_lightBuffers[currentFrame].Allocation.Mapping + 16);

	uint32_t* lightCountData = reinterpret_cast<uint32_t*>(
		_lightBuffers[currentFrame].Allocation.Mapping);

	glm::mat4* shadowTransforms = reinterpret_cast<glm::mat4*>(
		(char*)_lightBuffers[currentFrame].Allocation.Mapping + 1024);

	glm::mat4 shadowProj =
		glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 500.0f);

	for (auto& light : orderedLights) {
		if (!light.second->IsLightActive()) {
			continue;
		}

		lightDescriptors[selectedLights].Position =
			light.second->GetLightPosition();
		lightDescriptors[selectedLights].Color =
			light.second->GetLightColor();
		lightDescriptors[selectedLights].Direction =
			light.second->GetLightDirection();
		lightDescriptors[selectedLights].Type =
			(uint32_t)light.second->GetLightType();
		lightDescriptors[selectedLights].Angle =
			cos(glm::radians(light.second->GetLightAngle()));
		lightDescriptors[selectedLights].OuterAngle =
			cos(glm::radians(
			light.second->GetLightAngle() +
			light.second->GetLightAngleFade()));

		glm::vec3 lightPos = lightDescriptors[selectedLights].Position;

		shadowTransforms[selectedLights * 6] =
			shadowProj *
			glm::lookAt(
				lightPos,
				lightPos + glm::vec3( 1.0, 0.0, 0.0),
				glm::vec3(0.0,-1.0, 0.0));
		shadowTransforms[selectedLights * 6 + 1] =
			shadowProj *
			glm::lookAt(
				lightPos,
				lightPos + glm::vec3(-1.0, 0.0, 0.0),
				glm::vec3(0.0,-1.0, 0.0));
		shadowTransforms[selectedLights * 6 + 2] =
			shadowProj *
			glm::lookAt(
				lightPos,
				lightPos + glm::vec3( 0.0, 1.0, 0.0),
				glm::vec3(0.0, 0.0, 1.0));
		shadowTransforms[selectedLights * 6 + 3] =
			shadowProj *
			glm::lookAt(
				lightPos,
				lightPos + glm::vec3( 0.0,-1.0, 0.0),
				glm::vec3(0.0, 0.0,-1.0));
		shadowTransforms[selectedLights * 6 + 4] =
			shadowProj *
			glm::lookAt(
				lightPos,
				lightPos + glm::vec3( 0.0, 0.0, 1.0),
				glm::vec3(0.0,-1.0, 0.0));
		shadowTransforms[selectedLights * 6 + 5] =
			shadowProj *
			glm::lookAt(
				lightPos,
				lightPos + glm::vec3( 0.0, 0.0,-1.0),
				glm::vec3(0.0,-1.0, 0.0));

		++selectedLights;

		if (selectedLights >= _maxLightCount) {
			break;
		}
	}

	*lightCountData = selectedLights;

	// Shadow pass
	VkViewport shadowViewport{};
	shadowViewport.x = 0.0f;
	shadowViewport.y = 0.0f;
	shadowViewport.width = _shadowSize;
	shadowViewport.height = _shadowSize;
	shadowViewport.minDepth = 0.0f;
	shadowViewport.maxDepth = 1.0f;

	VkRect2D shadowScissor{};
	shadowScissor.offset = {0, 0};
	shadowScissor.extent.width = _shadowSize;
	shadowScissor.extent.height = _shadowSize;

	for (
		uint32_t lightIndex = 0;
		lightIndex < selectedLights;
		++lightIndex)
	{
		_shadowPipeline->RecordCommandBuffer(commandBuffer, lightIndex);

		vkCmdSetViewport(commandBuffer, 0, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 1, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 1, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 2, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 2, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 3, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 3, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 4, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 4, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 5, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 5, 1, &shadowScissor);

		std::set<Model*> holedModels;

		for (auto& model : _scene->DrawnScene.Models) {
			if (!model._IsDrawEnabled()) {
				continue;
			}

			if (model.DrawLight()) {
				continue;
			}

			if (model.GetColorMultiplier().a < 1.0f) {
				continue;
			}

			if (model.IsModelHoled()) {
				holedModels.insert(&model);
				continue;
			}

			mvp.Model = model.GetModelMatrix();
			mvp.InnerModel = model.GetModelInnerMatrix();

			if (_scene->ModelDescriptors.find(
				model.GetModels()[0]) ==
				_scene->ModelDescriptors.end())
			{
				continue;
			}

			auto& desc =
				_scene->ModelDescriptors[model.GetModels()[0]];

			if (desc.InstanceCount == 0) {
				continue;
			}

			VkBuffer vertexBuffers[] = {
				desc.VertexBuffer.Buffer,
				desc.InstanceBuffer.Buffer
			};

			VkDeviceSize offsets[] = {0, 0};
			vkCmdBindVertexBuffers(
				commandBuffer,
				0,
				2,
				vertexBuffers,
				offsets);

			vkCmdBindIndexBuffer(
				commandBuffer,
				desc.IndexBuffer.Buffer,
				0,
				VK_INDEX_TYPE_UINT32);

			vkCmdPushConstants(
				commandBuffer,
				_shadowPipeline->GetPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(MVP),
				&mvp);

			vkCmdPushConstants(
				commandBuffer,
				_shadowPipeline->GetPipelineLayout(),
				VK_SHADER_STAGE_GEOMETRY_BIT |
				VK_SHADER_STAGE_FRAGMENT_BIT,
				192,
				sizeof(uint32_t),
				&lightIndex);

			std::vector<VkDescriptorSet> descriptorSets = {
				_lightDescriptorSets[currentFrame]
			};

			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				_shadowPipeline->GetPipelineLayout(),
				0,
				descriptorSets.size(),
				descriptorSets.data(),
				0,
				nullptr);

			vkCmdDrawIndexed(
				commandBuffer,
				desc.IndexCount,
				desc.InstanceCount,
				0,
				0,
				0);
		}

		vkCmdEndRenderPass(commandBuffer);

		_shadowDiscardPipeline->RecordCommandBuffer(
			commandBuffer,
			lightIndex);

		vkCmdSetViewport(commandBuffer, 0, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 1, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 1, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 2, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 2, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 3, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 3, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 4, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 4, 1, &shadowScissor);

		vkCmdSetViewport(commandBuffer, 5, 1, &shadowViewport);
		vkCmdSetScissor(commandBuffer, 5, 1, &shadowScissor);

		for (auto& model : holedModels) {
			mvp.Model = model->GetModelMatrix();
			mvp.InnerModel = model->GetModelInnerMatrix();

			if (_scene->ModelDescriptors.find(
				model->GetModels()[0]) ==
				_scene->ModelDescriptors.end())
			{
				continue;
			}

			auto& desc =
				_scene->ModelDescriptors[model->GetModels()[0]];

			if (desc.InstanceCount == 0) {
				continue;
			}

			if (!_scene->Textures->CheckTexture(
				model->GetTextures()[0]))
			{
				continue;
			}

			VkBuffer vertexBuffers[] = {
				desc.VertexBuffer.Buffer,
				desc.InstanceBuffer.Buffer
			};

			VkDeviceSize offsets[] = {0, 0};
			vkCmdBindVertexBuffers(
				commandBuffer,
				0,
				2,
				vertexBuffers,
				offsets);

			vkCmdBindIndexBuffer(
				commandBuffer,
				desc.IndexBuffer.Buffer,
				0,
				VK_INDEX_TYPE_UINT32);

			vkCmdPushConstants(
				commandBuffer,
				_shadowDiscardPipeline->GetPipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(MVP),
				&mvp);

			vkCmdPushConstants(
				commandBuffer,
				_shadowDiscardPipeline->GetPipelineLayout(),
				VK_SHADER_STAGE_GEOMETRY_BIT |
				VK_SHADER_STAGE_FRAGMENT_BIT,
				192,
				sizeof(uint32_t),
				&lightIndex);

			auto& texDiff = _scene->Textures->GetTexture(
				model->GetTextures()[0]);

			std::vector<VkDescriptorSet> descriptorSets = {
				_lightDescriptorSets[currentFrame],
				texDiff.DescriptorSet
			};

			vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				_shadowDiscardPipeline->GetPipelineLayout(),
				0,
				descriptorSets.size(),
				descriptorSets.data(),
				0,
				nullptr);

			vkCmdDrawIndexed(
				commandBuffer,
				desc.IndexCount,
				desc.InstanceCount,
				0,
				0,
				0);
		}

		vkCmdEndRenderPass(commandBuffer);
	}

	// Object pass.
	_objectPipeline->RecordCommandBuffer(commandBuffer, 0);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	std::multimap<float, Model*> transparentModels;
	std::set<Model*> holedModels;

	for (auto& model : _scene->DrawnScene.Models) {
		if (!model._IsDrawEnabled()) {
			continue;
		}

		if (model.GetColorMultiplier().a < 1.0f) {
			float distance = glm::length(
				_scene->DrawnScene.CameraPosition -
				model.GetModelCenter());

			transparentModels.insert({distance, &model});

			continue;
		}

		if (model.IsModelHoled()) {
			holedModels.insert(&model);
			continue;
		}

		RecordObjectCommandBuffer(
			commandBuffer,
			imageIndex,
			currentFrame,
			&model,
			mvp,
			_objectPipeline);
	}

	vkCmdEndRenderPass(commandBuffer);

	_holedObjectPipeline->RecordCommandBuffer(commandBuffer, 0);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	for (auto& model : holedModels) {
		RecordObjectCommandBuffer(
			commandBuffer,
			imageIndex,
			currentFrame,
			model,
			mvp,
			_holedObjectPipeline);
	}

	vkCmdEndRenderPass(commandBuffer);

	_transparentObjectPipeline->RecordCommandBuffer(commandBuffer, 0);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	for (
		auto it = transparentModels.rbegin();
		it != transparentModels.rend();
		++it)
	{
		RecordObjectCommandBuffer(
			commandBuffer,
			imageIndex,
			currentFrame,
			it->second,
			mvp,
			_transparentObjectPipeline);
	}

	vkCmdEndRenderPass(commandBuffer);

	// Sprite pipeline.
	_spritePipeline->RecordCommandBuffer(commandBuffer, 0);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	std::multimap<float, Sprite*> orderedSprites;

	for (auto& sprite : _scene->DrawnScene.Sprites) {
		if (!sprite._IsDrawEnabled()) {
			continue;
		}

		orderedSprites.insert({
			glm::length(sprite.GetSpritePosition() -
				_scene->DrawnScene.CameraPosition),
			&sprite
		});
	}

	for (
		auto it = orderedSprites.rbegin();
		it != orderedSprites.rend();
		++it)
	{
		auto& sprite = *it;

		bool validDiffTexture =
			_scene->Textures->CheckTexture(
				sprite.second->GetTexture(0));

		bool validSpecTexture = sprite.second->GetTexCount() > 1 ?
			_scene->Textures->CheckTexture(
				sprite.second->GetTexture(1)) : true;

		if (!(validDiffTexture && validSpecTexture)) {
			continue;
		}

		SpriteDescriptor spriteDesc;

		spriteDesc.ProjView = mvp.ProjView;
		spriteDesc.TexCoords = sprite.second->GetSpriteTexCoords();
		spriteDesc.SpritePos = sprite.second->GetSpritePosition();
		spriteDesc.CameraPos = _scene->DrawnScene.CameraPosition;
		spriteDesc.SpriteUp = sprite.second->GetSpriteUp();
		spriteDesc.Size = sprite.second->GetSpriteSize();

		vkCmdPushConstants(
			commandBuffer,
			_spritePipeline->GetPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(SpriteDescriptor),
			&spriteDesc);

		vkCmdPushConstants(
			commandBuffer,
			_spritePipeline->GetPipelineLayout(),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			192,
			sizeof(glm::vec3),
			&_scene->DrawnScene.CameraPosition);

		uint32_t isLight = sprite.second->DrawLight() ? 1 : 0;

		vkCmdPushConstants(
			commandBuffer,
			_spritePipeline->GetPipelineLayout(),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			204,
			sizeof(uint32_t),
			&isLight);

		const glm::vec4 colorMultiplier =
			sprite.second->GetColorMultiplier();

		vkCmdPushConstants(
			commandBuffer,
			_spritePipeline->GetPipelineLayout(),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			208,
			sizeof(glm::vec4),
			&colorMultiplier);

		auto& texDiff = _scene->Textures->GetTexture(
			sprite.second->GetTexture(0));

		auto& texSpec = sprite.second->GetTexCount() > 1 ?
			_scene->Textures->GetTexture(
				sprite.second->GetTexture(1)) : texDiff;

		std::vector<VkDescriptorSet> descriptorSets = {
			texDiff.DescriptorSet,
			texSpec.DescriptorSet,
			_lightDescriptorSets[currentFrame]
		};

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			_spritePipeline->GetPipelineLayout(),
			0,
			descriptorSets.size(),
			descriptorSets.data(),
			0,
			nullptr);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);

	// Rectangle pipeline.
	_rectanglePipeline->RecordCommandBuffer(commandBuffer, 0, 0.0f);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	std::vector<glm::vec4> rectData(2);

	std::multimap<float, Rectangle*> orderedRectangles;

	for (auto& rectangle : _scene->DrawnScene.Rectangles) {
		if (!rectangle._IsDrawEnabled()) {
			continue;
		}

		orderedRectangles.insert({
			rectangle.GetRectangleDepth(),
			&rectangle
		});
	}

	for (auto& rect : orderedRectangles) {
		auto rectangle = rect.second;

		if (!_scene->Textures->CheckTexture(
			rectangle->GetTexture(0)))
		{
			continue;
		}

		rectData[0] = rectangle->GetRectanglePosition();
		rectData[1] = rectangle->GetRectangleTexCoords();

		vkCmdPushConstants(
			commandBuffer,
			_rectanglePipeline->GetPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(glm::vec4) * 2,
			rectData.data());

		glm::vec4 colorMultiplier =
			rectangle->GetColorMultiplier();

		vkCmdPushConstants(
			commandBuffer,
			_rectanglePipeline->GetPipelineLayout(),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			32,
			sizeof(glm::vec4),
			&colorMultiplier);

		auto& tex = _scene->Textures->GetTexture(
			rectangle->GetTexture(0));

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			_rectanglePipeline->GetPipelineLayout(),
			0,
			1,
			&tex.DescriptorSet,
			0,
			nullptr);

		vkCmdDraw(commandBuffer, 6, 1, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);

	// Postprocessing pipeline
	_postprocessingPipeline->RecordCommandBuffer(commandBuffer, imageIndex);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	float exposure[2];
	exposure[0] = 0.3;
	exposure[1] = 1.5;

	static uint32_t expIdx = 0;

	vkCmdPushConstants(
		commandBuffer,
		_postprocessingPipeline->GetPipelineLayout(),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(exposure),
		exposure);

	vkCmdPushConstants(
		commandBuffer,
		_postprocessingPipeline->GetPipelineLayout(),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		8,
		sizeof(uint32_t),
		&expIdx);

	expIdx = (expIdx + 1) % 2;

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		_postprocessingPipeline->GetPipelineLayout(),
		0,
		1,
		&_hdrDescriptorSet,
		0,
		nullptr);

	vkCmdDraw(commandBuffer, 6, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	// End buffer
	res = vkEndCommandBuffer(commandBuffer);
	
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to end command buffer.");
	}
}

void Swapchain::RecordObjectCommandBuffer(
	VkCommandBuffer commandBuffer,
	uint32_t imageIndex,
	uint32_t currentFrame,
	Model* model,
	MVP mvp,
	Pipeline* pipeline)
{
	mvp.Model = model->GetModelMatrix();
	mvp.InnerModel = model->GetModelInnerMatrix();

	if (_scene->ModelDescriptors.find(
		model->GetModels()[0]) ==
		_scene->ModelDescriptors.end())
	{
		return;
	}

	auto& desc = _scene->ModelDescriptors[model->GetModels()[0]];

	if (desc.InstanceCount == 0) {
		return;
	}

	auto& textures = model->GetTextures();

	bool validDiffTexture = _scene->Textures->CheckTexture(textures[0]);

	bool validSpecTexture = textures.size() > 1 ?
		_scene->Textures->CheckTexture(textures[1]) :
		true;

	if (!(validDiffTexture && validSpecTexture)) {
		return;
	}

	VkBuffer vertexBuffers[] = {
		desc.VertexBuffer.Buffer,
		desc.InstanceBuffer.Buffer
	};

	VkDeviceSize offsets[] = {0, 0};
	vkCmdBindVertexBuffers(
		commandBuffer,
		0,
		2,
		vertexBuffers,
		offsets);

	vkCmdBindIndexBuffer(
		commandBuffer,
		desc.IndexBuffer.Buffer,
		0,
		VK_INDEX_TYPE_UINT32);

	vkCmdPushConstants(
		commandBuffer,
		pipeline->GetPipelineLayout(),
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(MVP),
		&mvp);

	vkCmdPushConstants(
		commandBuffer,
		pipeline->GetPipelineLayout(),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		192,
		sizeof(glm::vec3),
		&_scene->DrawnScene.CameraPosition);

	uint32_t isLight = model->DrawLight() ? 1 : 0;

	vkCmdPushConstants(
		commandBuffer,
		pipeline->GetPipelineLayout(),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		204,
		sizeof(uint32_t),
		&isLight);

	glm::vec4 colorMultiplier =
		model->GetColorMultiplier();

	vkCmdPushConstants(
		commandBuffer,
		pipeline->GetPipelineLayout(),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		208,
		sizeof(glm::vec4),
		&colorMultiplier);

	auto& texDiff = _scene->Textures->GetTexture(textures[0]);

	auto& texSpec = textures.size() > 1 ?
		_scene->Textures->GetTexture(textures[1]) :
		texDiff;

	std::vector<VkDescriptorSet> descriptorSets = {
		texDiff.DescriptorSet,
		texSpec.DescriptorSet,
		_lightDescriptorSets[currentFrame]
	};

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->GetPipelineLayout(),
		0,
		descriptorSets.size(),
		descriptorSets.data(),
		0,
		nullptr);

	vkCmdDrawIndexed(
		commandBuffer,
		desc.IndexCount,
		desc.InstanceCount,
		0,
		0,
		0);
}

void Swapchain::MainLoop() {
	Logger::Verbose() << "Video main loop called.";

	_work = true;

	uint32_t frameCount = 0;
	auto refTime = std::chrono::high_resolution_clock::now();

	while (!glfwWindowShouldClose(_window) && _work) {
		glfwPollEvents();
		DrawFrame();

		++frameCount;
		auto currTime = std::chrono::high_resolution_clock::now();
		uint32_t dur = std::chrono::duration_cast
			<std::chrono::milliseconds>(currTime - refTime).count();

		if (dur >= 1000) {
			refTime = currTime;

			if (frameCount < 58) {
				Logger::Warning() << "FPS " << frameCount;
			}

			frameCount = 0;
		}
	}

	vkDeviceWaitIdle(_device);
}

void Swapchain::Stop()
{
	_work = false;
}

void Swapchain::DrawFrame()
{
	vkWaitForFences(
		_device,
		1,
		&_inFlightFences[_currentFrame],
		VK_TRUE,
		UINT64_MAX);

	uint32_t imageIndex;

	VkResult res = vkAcquireNextImageKHR(
		_device,
		_swapchain,
		UINT64_MAX,
		_imageAvailableSemaphores[_currentFrame],
		VK_NULL_HANDLE,
		&imageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
		vkDeviceWaitIdle(_device);
		Destroy();
		Create();
		return;
	} else if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to acquire swap chain image.");
	}

	vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);
	vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);

	auto it = _scene->DeletedModelDescriptors.begin();

	while (it != _scene->DeletedModelDescriptors.end()) {
		if (it->MarkedFrameIndex == _currentFrame) {
			ModelDescriptor::DestroyModelDescriptor(
				*it,
				_device,
				_memorySystem);

			auto remIt = it;
			++it;
			_scene->DeletedModelDescriptors.erase(remIt);
		} else {
			if (it->MarkedFrameIndex == -1) {
				it->MarkedFrameIndex = _currentFrame;
			}

			++it;
		}
	}

	RecordCommandBuffer(
		_commandBuffers[_currentFrame],
		imageIndex,
		_currentFrame);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {
		_imageAvailableSemaphores[_currentFrame]
	};

	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];

	VkSemaphore signalSemaphores[] = {
		_renderFinishedSemaphores[_currentFrame]
	};

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	_graphicsQueue->Mutex.lock();
	res = vkQueueSubmit(
		_graphicsQueue->Queue,
		1,
		&submitInfo,
		_inFlightFences[_currentFrame]);
	_graphicsQueue->Mutex.unlock();

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to submit draw command buffer.");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = {_swapchain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(_presentQueue, &presentInfo);

	_currentFrame = (_currentFrame + 1) % _maxFramesInFlight;
}

void Swapchain::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	_commandBuffers.resize(_maxFramesInFlight);
	_imageAvailableSemaphores.resize(_maxFramesInFlight);
	_renderFinishedSemaphores.resize(_maxFramesInFlight);
	_inFlightFences.resize(_maxFramesInFlight);

	for (size_t i = 0; i < _maxFramesInFlight; ++i) {
		VkResult res = vkCreateSemaphore(
			_device,
			&semaphoreInfo,
			nullptr, &_imageAvailableSemaphores[i]);

		if (res != VK_SUCCESS) {
			throw std::runtime_error("Failed to create semaphore.");
		}

		res = vkCreateSemaphore(
			_device,
			&semaphoreInfo,
			nullptr,
			&_renderFinishedSemaphores[i]);

		if (res != VK_SUCCESS) {
			throw std::runtime_error("Failed to create semaphore.");
		}

		res = vkCreateFence(
			_device,
			&fenceInfo,
			nullptr,
			&_inFlightFences[i]);

		if (res != VK_SUCCESS) {
			throw std::runtime_error("Failed to create fence.");
		}
	}
}

void Swapchain::DestroySyncObjects()
{
	for (size_t i = 0; i < _maxFramesInFlight; ++i) {
		vkDestroySemaphore(
			_device,
			_imageAvailableSemaphores[i],
			nullptr);

		vkDestroySemaphore(
			_device,
			_renderFinishedSemaphores[i],
			nullptr);

		vkDestroyFence(_device, _inFlightFences[i], nullptr);
	}
}
