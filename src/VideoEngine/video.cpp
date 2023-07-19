#include "video.h"

#include <vector>
#include <string.h>
#include <set>

#include "../Logger/logger.h"

Video::Video(
	int width,
	int height,
	std::string name,
	std::string applicationName)
	: _window(width, height, name)
{
	VkInstanceHandler::SetApplicationName(applicationName);
	VkInstanceHandler::IncRef();

	_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	CreateSurface();
	SelectPhysicalDevice();
	CreateDevice();
	CreateCommandPools();
	CreateDescriptorSetLayout();

	CreateSwapchain();
}

Video::~Video()
{
	DestroySwapchain();

	RemoveAllModels();

	DestroyDescriptorSetLayout();
	DestroyCommandPools();
	DestroyDevice();
	DestroySurface();

	VkInstanceHandler::DecRef();
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

	Logger::Verbose(deviceProperties.deviceName);

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

	for (auto bit : bits) {
		if (counts & bit) {
			return bit;
		}
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
		&_models,
		&_viewMatrix,
		&_fov,
		_descriptorSetLayout);

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
	_models[model] = CreateModelDescriptor(model);
	model->SetModelReady(true);
}

void Video::RemoveModel(Model* model)
{
	model->SetModelReady(false);
	vkQueueWaitIdle(_graphicsQueue);

	DestroyModelDescriptor(_models[model]);
	_models.erase(model);
}

void Video::RemoveAllModels()
{
	vkQueueWaitIdle(_graphicsQueue);

	for (auto& model : _models) {
		model.first->SetModelReady(false);
		DestroyModelDescriptor(model.second);
	}

	_models.clear();
}

ModelDescriptor Video::CreateModelDescriptor(Model* model)
{
	ModelDescriptor descriptor;

	// Vertex buffer creation.
	auto& vertices = model->GetModelVertices();
	auto& texCoords = model->GetTexCoords();

	descriptor.VertexBuffer = BufferHelper::CreateBuffer(
		_device,
		sizeof(ModelDescriptor::Vertex) * vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_memorySystem,
		&_deviceSupport);

	BufferHelper::Buffer stagingBuffer = BufferHelper::CreateBuffer(
		_device,
		sizeof(ModelDescriptor::Vertex) * vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		_memorySystem,
		&_deviceSupport);

	ModelDescriptor::Vertex* data;
	vkMapMemory(
		_device,
		stagingBuffer.Allocation.Memory,
		stagingBuffer.Allocation.Offset,
		stagingBuffer.Allocation.Size,
		0,
		reinterpret_cast<void**>(&data));

	for (size_t i = 0; i < vertices.size(); ++i) {
		data[i].Pos = vertices[i];
		data[i].TexCoord = texCoords[i];
	}

	vkUnmapMemory(
		_device,
		stagingBuffer.Allocation.Memory);

	BufferHelper::CopyBuffer(
		stagingBuffer,
		descriptor.VertexBuffer,
		_transferCommandPool,
		_graphicsQueue);

	BufferHelper::DestroyBuffer(
		_device,
		stagingBuffer,
		_memorySystem);

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

	stagingBuffer = BufferHelper::CreateBuffer(
		_device,
		sizeof(uint32_t) * indices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		_memorySystem,
		&_deviceSupport);

	uint32_t* indexData;
	vkMapMemory(
		_device,
		stagingBuffer.Allocation.Memory,
		stagingBuffer.Allocation.Offset,
		stagingBuffer.Allocation.Size,
		0,
		reinterpret_cast<void**>(&indexData));

	memcpy(indexData, indices.data(), sizeof(uint32_t) * indices.size());

	vkUnmapMemory(
		_device,
		stagingBuffer.Allocation.Memory);

	BufferHelper::CopyBuffer(
		stagingBuffer,
		descriptor.IndexBuffer,
		_transferCommandPool,
		_graphicsQueue);

	BufferHelper::DestroyBuffer(
		_device,
		stagingBuffer,
		_memorySystem);

	descriptor.IndexCount = indices.size();

	// Texture image.
	descriptor.TextureImage = CreateTextureImage(model);
	descriptor.TextureImageView = ImageHelper::CreateImageView(
		_device,
		descriptor.TextureImage.Image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		1);
	descriptor.TextureSampler = CreateTextureSampler(0.0f);

	CreateDescriptors(&descriptor);

	return descriptor;
}

void Video::DestroyModelDescriptor(ModelDescriptor descriptor)
{
	DestroyDescriptors(&descriptor);

	BufferHelper::DestroyBuffer(
		_device,
		descriptor.VertexBuffer,
		_memorySystem);

	BufferHelper::DestroyBuffer(
		_device,
		descriptor.IndexBuffer,
		_memorySystem);

	DestroyTextureSampler(descriptor.TextureSampler);

	ImageHelper::DestroyImageView(
		_device,
		descriptor.TextureImageView);

	ImageHelper::DestroyImage(
		_device,
		descriptor.TextureImage,
		_memorySystem);
}

ImageHelper::Image Video::CreateTextureImage(Model* model)
{
	uint32_t texWidth = model->GetTexWidth();
	uint32_t texHeight = model->GetTexHeight();

	uint32_t imageSize = texWidth * texHeight * 4;

	ImageHelper::Image textureImage = ImageHelper::CreateImage(
		_device,
		texWidth,
		texHeight,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_memorySystem,
		&_deviceSupport);

	BufferHelper::Buffer stagingBuffer = BufferHelper::CreateBuffer(
		_device,
		imageSize,
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
		_transferCommandPool,
		_graphicsQueue);

	ImageHelper::CopyBufferToImage(
		stagingBuffer,
		textureImage,
		texWidth,
		texHeight,
		_transferCommandPool,
		_graphicsQueue);

	BufferHelper::DestroyBuffer(
		_device,
		stagingBuffer,
		_memorySystem);

	ImageHelper::ChangeImageLayout(
		textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		_transferCommandPool,
		_graphicsQueue);

	return textureImage;
}

VkSampler Video::CreateTextureSampler(float mipLevels)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(_physicalDevice, &properties);

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = mipLevels;

	VkSampler sampler;

	VkResult res = vkCreateSampler(
		_device,
		&samplerInfo,
		nullptr,
		&sampler);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture sampler.");
	}

	return sampler;
}

void Video::DestroyTextureSampler(VkSampler sampler)
{
	vkDestroySampler(_device, sampler, nullptr);
}

void Video::CreateDescriptors(ModelDescriptor* descriptor)
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

void Video::DestroyDescriptors(ModelDescriptor* descriptor)
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

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
		samplerLayoutBinding
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
