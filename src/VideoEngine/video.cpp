#include "video.h"

#include <vector>
#include <set>
#include <cstring>

#include "../Logger/logger.h"

Video::Video(
	int width,
	int height,
	std::string name,
	std::string applicationName,
	GraphicsSettings* settings)
	: _window(width, height, name)
{
	_scene.SceneMutex = nullptr;
	if (settings) {
		_settings = *settings;
		_settingsValid = true;
	} else {
		_settingsValid = false;
	}

	VkInstanceHandler::SetApplicationName(applicationName);
	VkInstanceHandler::IncRef();

	_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	CreateSurface();
	SelectPhysicalDevice();
	CreateDevice();
	_inputControl = new InputControl(_window.GetWindow());
	CreateCommandPools();
	CreateDescriptorSetLayout();

	CreateSwapchain();
}

Video::~Video()
{
	DestroySwapchain();

	RemoveAllModels();
	DestroySkybox();

	DestroyDescriptorSetLayout();
	DestroyCommandPools();
	delete _inputControl;
	DestroyDevice();
	DestroySurface();

	VkInstanceHandler::DecRef();

	_scene.SceneMutex = nullptr;
}

void Video::CreateSurface()
{
	VkResult res = glfwCreateWindowSurface(
		VkInstanceHandler::GetInstance(),
		_window.GetWindow(),
		nullptr,
		&_surface);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface.");
	}

	_deviceSupport.SetSurface(_surface);
}

void Video::DestroySurface()
{
	vkDestroySurfaceKHR(
		VkInstanceHandler::GetInstance(),
		_surface,
		nullptr);
}

void Video::SelectPhysicalDevice()
{
	_physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(
		VkInstanceHandler::GetInstance(),
		&deviceCount,
		nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error(
			"Failed to find device with Vulkan support.");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(
		VkInstanceHandler::GetInstance(),
		&deviceCount,
		devices.data());

	for (const auto& device : devices) {
		if (IsDeviceSuitable(device)) {
			_physicalDevice = device;
			_msaaSamples = GetMaxSampleCount();

			if (_msaaSamples == VK_SAMPLE_COUNT_64_BIT) {
				Logger::Verbose() << "MSAA 64";
			} else if (_msaaSamples == VK_SAMPLE_COUNT_32_BIT) {
				Logger::Verbose() << "MSAA 32";
			} else if (_msaaSamples == VK_SAMPLE_COUNT_16_BIT) {
				Logger::Verbose() << "MSAA 16";
			} else if (_msaaSamples == VK_SAMPLE_COUNT_8_BIT) {
				Logger::Verbose() << "MSAA 8";
			} else if (_msaaSamples == VK_SAMPLE_COUNT_4_BIT) {
				Logger::Verbose() << "MSAA 4";
			} else if (_msaaSamples == VK_SAMPLE_COUNT_2_BIT) {
				Logger::Verbose() << "MSAA 2";
			} else if (_msaaSamples == VK_SAMPLE_COUNT_1_BIT) {
				Logger::Verbose() << "MSAA 1";
			}

			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Failed to find suitable device.");
	}
}

bool Video::IsDeviceSuitable(VkPhysicalDevice device)
{
	_deviceSupport.SetPhysicalDevice(device);

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	Logger::Verbose() << deviceProperties.deviceName;
	Logger::Verbose() << "Push constant size: " <<
		deviceProperties.limits.maxPushConstantsSize;

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapchainAdequate = false;

	if (extensionsSupported) {
		PhysicalDeviceSupport::SwapchainSupportDetails
			swapchainSupport =
			_deviceSupport.QuerySwapchainSupport();

		swapchainAdequate = !swapchainSupport.formats.empty() &&
			!swapchainSupport.presentModes.empty();
	}

	bool res = deviceProperties.deviceType ==
		VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		extensionsSupported &&
		swapchainAdequate &&
		deviceFeatures.geometryShader &&
		deviceFeatures.samplerAnisotropy &&
		deviceFeatures.shaderUniformBufferArrayDynamicIndexing &&
		_deviceSupport.FindQueueFamilies().graphicsFamily.has_value() &&
		_deviceSupport.FindQueueFamilies().presentFamily.has_value();

	return res;
}

VkSampleCountFlagBits Video::GetMaxSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(
		_physicalDevice,
		&physicalDeviceProperties);

	VkSampleCountFlags counts =
		physicalDeviceProperties.limits.framebufferColorSampleCounts &
		physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	std::vector<VkSampleCountFlagBits> bits = {
		VK_SAMPLE_COUNT_64_BIT,
		VK_SAMPLE_COUNT_32_BIT,
		VK_SAMPLE_COUNT_16_BIT,
		VK_SAMPLE_COUNT_8_BIT,
		VK_SAMPLE_COUNT_4_BIT,
		VK_SAMPLE_COUNT_2_BIT
	};

	uint32_t maxSamples = _settingsValid ? _settings.MsaaLimit : 64;

	uint32_t cSamples = 64;

	for (auto bit : bits) {
		if ((counts & bit) && cSamples <= maxSamples) {
			return bit;
		}

		cSamples /= 2;
	}

	return VK_SAMPLE_COUNT_1_BIT;
}

bool Video::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(
		device,
		nullptr,
		&extensionCount,
		nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		device,
		nullptr,
		&extensionCount,
		availableExtensions.data());

	for (const char* requiredExtension : _deviceExtensions) {
		bool extensionFound = false;

		for (const auto& extension : availableExtensions) {
			int cmp = strcmp(
				requiredExtension,
				extension.extensionName);

			if (cmp == 0) {
				extensionFound = true;
				break;
			}
		}

		if (!extensionFound) {
			return false;
		}
	}

	return true;
}

void Video::CreateDevice()
{
	_deviceSupport.SetPhysicalDevice(_physicalDevice);

	PhysicalDeviceSupport::QueueFamilyIndices indices =
		_deviceSupport.FindQueueFamilies();

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType =
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount =
		static_cast<uint32_t>(queueCreateInfos.size());
	deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.enabledExtensionCount =
		static_cast<uint32_t>(_deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = _deviceExtensions.data();
	deviceInfo.enabledLayerCount = 0;

	VkResult res = vkCreateDevice(
		_physicalDevice,
		&deviceInfo,
		nullptr,
		&_device);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create device.");
	}

	vkGetDeviceQueue(
		_device,
		indices.graphicsFamily.value(),
		0,
		&_graphicsQueue);

	vkGetDeviceQueue(
		_device,
		indices.presentFamily.value(),
		0,
		&_presentQueue);

	_memorySystem = new MemorySystem(_device);
}

void Video::DestroyDevice()
{
	delete _memorySystem;
	vkDestroyDevice(_device, nullptr);
}

void Video::CreateCommandPools()
{
	PhysicalDeviceSupport::QueueFamilyIndices indices =
		_deviceSupport.FindQueueFamilies();

	_transferCommandPool = new CommandPool(
		_device,
		indices.graphicsFamily.value(),
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
}

void Video::DestroyCommandPools()
{
	delete _transferCommandPool;
}

void Video::CreateSwapchain()
{
	_swapchain = new Swapchain(
		_device,
		_surface,
		_window.GetWindow(),
		&_deviceSupport,
		_memorySystem,
		_msaaSamples,
		_graphicsQueue,
		_presentQueue,
		&_scene,
		_descriptorSetLayout,
		10,
		256);

	_swapchain->Create();
}

void Video::DestroySwapchain()
{
	_swapchain->Destroy();
	delete _swapchain;
}

void Video::MainLoop()
{
	_swapchain->MainLoop();
}

void Video::Stop()
{
	_swapchain->Stop();
}

void Video::RegisterModel(Model* model)
{
	auto descriptor = CreateModelDescriptor(model);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	_scene.Models[model] = descriptor;

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}

	model->_SetDrawReady(true);
}

void Video::RemoveModel(Model* model)
{
	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	model->_SetDrawReady(false);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}

	vkQueueWaitIdle(_graphicsQueue);

	DestroyModelDescriptor(_scene.Models[model]);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	_scene.Models.erase(model);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}
}

void Video::RemoveAllModels()
{
	vkQueueWaitIdle(_graphicsQueue);

	for (auto& model : _scene.Models) {
		model.first->_SetDrawReady(false);
		DestroyModelDescriptor(model.second);
	}

	_scene.Models.clear();
}

ModelDescriptor Video::CreateModelDescriptor(Model* model)
{
	ModelDescriptor descriptor;

	// Vertex buffer creation.
	auto& vertices = model->GetModelVertices();
	auto& texCoords = model->GetModelTexCoords();
	auto& normals = model->GetModelNormals();

	std::vector<ModelDescriptor::Vertex> vertexData(vertices.size());

	descriptor.VertexBuffer = BufferHelper::CreateBuffer(
		_device,
		sizeof(ModelDescriptor::Vertex) * vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_memorySystem,
		&_deviceSupport);

	for (size_t i = 0; i < vertices.size(); ++i) {
		vertexData[i].Pos = vertices[i];
		vertexData[i].TexCoord = texCoords[i];
		vertexData[i].Normal = normals[i];
	}

	BufferHelper::LoadDataToBuffer(
		_device,
		descriptor.VertexBuffer,
		vertexData.data(),
		vertexData.size() * sizeof(ModelDescriptor::Vertex),
		_memorySystem,
		&_deviceSupport,
		_transferCommandPool,
		_graphicsQueue);

	descriptor.VertexCount = vertices.size();

	// Index buffer creation.
	auto& indices = model->GetModelIndices();

	descriptor.IndexBuffer = BufferHelper::CreateBuffer(
		_device,
		sizeof(uint32_t) * indices.size(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_memorySystem,
		&_deviceSupport);

	BufferHelper::LoadDataToBuffer(
		_device,
		descriptor.IndexBuffer,
		indices.data(),
		indices.size() * sizeof(uint32_t),
		_memorySystem,
		&_deviceSupport,
		_transferCommandPool,
		_graphicsQueue);

	descriptor.IndexCount = indices.size();

	// Instance buffer.
	auto& instances = model->GetModelInstances();

	descriptor.InstanceBuffer = BufferHelper::CreateBuffer(
		_device,
		sizeof(glm::mat4) * instances.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_memorySystem,
		&_deviceSupport);

	BufferHelper::LoadDataToBuffer(
		_device,
		descriptor.InstanceBuffer,
		instances.data(),
		instances.size() * sizeof(glm::mat4),
		_memorySystem,
		&_deviceSupport,
		_transferCommandPool,
		_graphicsQueue);

	descriptor.InstanceCount = instances.size();

	// Texture image.
	uint32_t mipLevels;
	descriptor.TextureImage = CreateTextureImage(model, mipLevels);
	descriptor.TextureImageView = ImageHelper::CreateImageView(
		_device,
		descriptor.TextureImage.Image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		mipLevels);
	descriptor.TextureSampler = ImageHelper::CreateImageSampler(
		_device,
		_physicalDevice,
		mipLevels);

	CreateDescriptorSets(&descriptor);

	return descriptor;
}

void Video::DestroyModelDescriptor(ModelDescriptor descriptor)
{
	DestroyDescriptorSets(&descriptor);

	BufferHelper::DestroyBuffer(
		_device,
		descriptor.VertexBuffer,
		_memorySystem);

	BufferHelper::DestroyBuffer(
		_device,
		descriptor.IndexBuffer,
		_memorySystem);

	BufferHelper::DestroyBuffer(
		_device,
		descriptor.InstanceBuffer,
		_memorySystem);

	ImageHelper::DestroyImageSampler(_device, descriptor.TextureSampler);

	ImageHelper::DestroyImageView(
		_device,
		descriptor.TextureImageView);

	ImageHelper::DestroyImage(
		_device,
		descriptor.TextureImage,
		_memorySystem);
}

void Video::RegisterRectangle(Rectangle* rectangle)
{
	auto descriptor = CreateRectangleDescriptor(rectangle);

	rectangle->SetRectangleScreenRatio(_swapchain->GetScreenRatio());

	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	_scene.Rectangles[rectangle] = descriptor;

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}

	rectangle->_SetDrawReady(true);
}

void Video::RemoveRectangle(Rectangle* rectangle)
{
	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	rectangle->_SetDrawReady(false);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}

	vkQueueWaitIdle(_graphicsQueue);

	DestroyRectangleDescriptor(_scene.Rectangles[rectangle]);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	_scene.Rectangles.erase(rectangle);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}
}

void Video::RemoveAllRectangles()
{
	vkQueueWaitIdle(_graphicsQueue);

	for (auto& rectangle : _scene.Rectangles) {
		rectangle.first->_SetDrawReady(false);
		DestroyRectangleDescriptor(rectangle.second);
	}

	_scene.Rectangles.clear();
}

ModelDescriptor Video::CreateRectangleDescriptor(Rectangle* rectangle)
{
	ModelDescriptor descriptor;

	// Texture image.
	uint32_t mipLevels;
	descriptor.TextureImage = CreateTextureImage(rectangle, mipLevels);
	descriptor.TextureImageView = ImageHelper::CreateImageView(
		_device,
		descriptor.TextureImage.Image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		mipLevels);
	descriptor.TextureSampler = ImageHelper::CreateImageSampler(
		_device,
		_physicalDevice,
		mipLevels);

	CreateDescriptorSets(&descriptor);

	return descriptor;
}

void Video::DestroyRectangleDescriptor(ModelDescriptor descriptor)
{
	DestroyDescriptorSets(&descriptor);

	ImageHelper::DestroyImageSampler(_device, descriptor.TextureSampler);

	ImageHelper::DestroyImageView(
		_device,
		descriptor.TextureImageView);

	ImageHelper::DestroyImage(
		_device,
		descriptor.TextureImage,
		_memorySystem);
}

void Video::CreateSkybox(
	uint32_t texWidth,
	uint32_t texHeight,
	const std::vector<uint8_t>& texData)
{
	DestroySkybox();

	uint32_t mipLevels;
	uint32_t layerCount = 6;

	// Texture layout transformation.
	texWidth /= layerCount;
	std::vector<uint8_t> texDataTransformed(texData.size(), 100);

	for (uint32_t layer = 0; layer < layerCount; ++layer) {
		for (uint32_t y = 0; y < texHeight; ++y) {
			for (uint32_t x = 0; x < texWidth; ++x) {
				for (uint32_t c = 0; c < 4; ++c) {
					texDataTransformed[
						(texWidth * texHeight * layer +
						texWidth * y +
						x) * 4 + c] =
					texData[(texWidth * layer +
						x +
						texWidth * layerCount * y) *
						4 + c];
				}
			}
		}
	}

	// 1.
	ImageHelper::RotateClockWise(
		texDataTransformed.data(),
		texWidth,
		texHeight);
	ImageHelper::FlipHorizontally(
		texDataTransformed.data(),
		texHeight,
		texWidth);

	// 2.
	ImageHelper::FlipHorizontally(
		texDataTransformed.data() + texWidth * texHeight * 4,
		texHeight,
		texWidth);

	// 3.
	ImageHelper::RotateCounterClockWise(
		texDataTransformed.data() + texWidth * texHeight * 4 * 2,
		texWidth,
		texHeight);
	ImageHelper::FlipHorizontally(
		texDataTransformed.data() + texWidth * texHeight * 4 * 2,
		texHeight,
		texWidth);

	// 4.
	ImageHelper::FlipVertically(
		texDataTransformed.data() + texWidth * texHeight * 4 * 3,
		texHeight,
		texWidth);

	// Ordering.
	ImageHelper::Swap(
		texDataTransformed.data() + texWidth * texHeight * 4 * 2,
		texDataTransformed.data() + texWidth * texHeight * 4,
		texWidth,
		texHeight);
	ImageHelper::Swap(
		texDataTransformed.data() + texWidth * texHeight * 4 * 3,
		texDataTransformed.data() + texWidth * texHeight * 4 * 2,
		texWidth,
		texHeight);
	ImageHelper::Swap(
		texDataTransformed.data() + texWidth * texHeight * 4 * 4,
		texDataTransformed.data() + texWidth * texHeight * 4 * 5,
		texWidth,
		texHeight);

	_scene.skybox.SetTexWidth(texWidth);
	_scene.skybox.SetTexHeight(texHeight);
	_scene.skybox.SetTexData(texDataTransformed);

	_scene.skybox.Descriptor.TextureImage = CreateTextureImage(
		&_scene.skybox,
		mipLevels,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		layerCount);
	_scene.skybox.Descriptor.TextureImageView =
		ImageHelper::CreateImageView(
			_device,
			_scene.skybox.Descriptor.TextureImage.Image,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT,
			mipLevels,
			VK_IMAGE_VIEW_TYPE_CUBE,
			layerCount);
	_scene.skybox.Descriptor.TextureSampler =
		ImageHelper::CreateImageSampler(
			_device,
			_physicalDevice,
			mipLevels);
	CreateDescriptorSets(&_scene.skybox.Descriptor);

	_scene.skybox._SetDrawReady(true);
	_scene.skybox.SetDrawEnabled(true);
}

void Video::DestroySkybox()
{
	if (!_scene.skybox.IsDrawEnabled())
	{
		return;
	}

	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	_scene.skybox._SetDrawReady(false);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}

	vkQueueWaitIdle(_graphicsQueue);

	DestroyDescriptorSets(&_scene.skybox.Descriptor);

	ImageHelper::DestroyImageSampler(
		_device,
		_scene.skybox.Descriptor.TextureSampler);

	ImageHelper::DestroyImageView(
		_device,
		_scene.skybox.Descriptor.TextureImageView);

	ImageHelper::DestroyImage(
		_device,
		_scene.skybox.Descriptor.TextureImage,
		_memorySystem);

	_scene.skybox.SetTexData(std::vector<uint8_t>());
}

ImageHelper::Image Video::CreateTextureImage(
	Texturable* model,
	uint32_t& mipLevels,
	VkImageCreateFlagBits flags,
	uint32_t layerCount)
{
	uint32_t texWidth = model->GetTexWidth();
	uint32_t texHeight = model->GetTexHeight();

	uint32_t imageSize = texWidth * texHeight * 4;

	mipLevels = static_cast<uint32_t>(
		std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (layerCount > 1) {
		mipLevels = 1;
	}

	ImageHelper::Image textureImage = ImageHelper::CreateImage(
		_device,
		texWidth,
		texHeight,
		mipLevels,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_memorySystem,
		&_deviceSupport,
		flags,
		layerCount);

	BufferHelper::Buffer stagingBuffer = BufferHelper::CreateBuffer(
		_device,
		imageSize * layerCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		_memorySystem,
		&_deviceSupport);

	uint32_t* imageData;
	vkMapMemory(
		_device,
		stagingBuffer.Allocation.Memory,
		stagingBuffer.Allocation.Offset,
		stagingBuffer.Allocation.Size,
		0,
		reinterpret_cast<void**>(&imageData));

	auto& pixels = model->GetTexData();
	memcpy(imageData, pixels.data(), pixels.size());

	vkUnmapMemory(
		_device,
		stagingBuffer.Allocation.Memory);

	ImageHelper::ChangeImageLayout(
		textureImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		mipLevels,
		_transferCommandPool,
		_graphicsQueue,
		layerCount);

	ImageHelper::CopyBufferToImage(
		stagingBuffer,
		textureImage,
		texWidth,
		texHeight,
		_transferCommandPool,
		_graphicsQueue,
		layerCount);

	BufferHelper::DestroyBuffer(
		_device,
		stagingBuffer,
		_memorySystem);

	if (layerCount == 1) {
		GenerateMipmaps(
			textureImage,
			texWidth,
			texHeight,
			mipLevels);
	} else {
		ImageHelper::ChangeImageLayout(
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			mipLevels,
			_transferCommandPool,
			_graphicsQueue,
			layerCount);
	}

	return textureImage;
}

void Video::GenerateMipmaps(
	ImageHelper::Image image,
	uint32_t width,
	uint32_t height,
	uint32_t mipLevels)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(
		_physicalDevice,
		image.Format,
		&formatProperties);

	if (!(formatProperties.optimalTilingFeatures &
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error(
			"Image format does not support linear blitting.");
	}

	VkCommandBuffer commandBuffer =
		_transferCommandPool->StartOneTimeBuffer();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image.Image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = static_cast<int32_t>(width);
	int32_t mipHeight = static_cast<int32_t>(height);

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = {
			mipWidth > 1 ? mipWidth / 2 : 1,
			mipHeight > 1 ? mipHeight / 2 : 1,
			1
		};

		blit.dstSubresource.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			commandBuffer,
			image.Image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image.Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		if (mipWidth > 1) {
			mipWidth /= 2;
		}

		if (mipHeight > 1) {
			mipHeight /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier);

	_transferCommandPool->EndOneTimeBuffer(commandBuffer, _graphicsQueue);
}

void Video::CreateDescriptorSets(ModelDescriptor* descriptor)
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	VkResult res = vkCreateDescriptorPool(
		_device,
		&poolInfo,
		nullptr,
		&descriptor->DescriptorPool);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool.");
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptor->DescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_descriptorSetLayout;

	res = vkAllocateDescriptorSets(
		_device,
		&allocInfo,
		&descriptor->DescriptorSet);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor set.");
	}


	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = descriptor->TextureImageView;
	imageInfo.sampler = descriptor->TextureSampler;

	VkWriteDescriptorSet descriptorWrite{};

	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptor->DescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(
		_device,
		1,
		&descriptorWrite,
		0,
		nullptr);
}

void Video::DestroyDescriptorSets(ModelDescriptor* descriptor)
{
	vkDestroyDescriptorPool(_device, descriptor->DescriptorPool, nullptr);
}

void Video::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
		samplerLayoutBinding,
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult res = vkCreateDescriptorSetLayout(
		_device,
		&layoutInfo,
		nullptr,
		&_descriptorSetLayout);
	
	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to create descriptor set layout.");
	}
}

void Video::DestroyDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
}

void Video::RegisterLight(Light* light)
{
	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	_scene.Lights.insert(light);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}
}

void Video::RemoveLight(Light* light)
{
	if (_scene.SceneMutex) {
		_scene.SceneMutex->lock();
	}

	_scene.Lights.erase(light);

	if (_scene.SceneMutex) {
		_scene.SceneMutex->unlock();
	}
}
