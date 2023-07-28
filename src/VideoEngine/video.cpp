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
	_scene.Textures = new TextureHandler(
		_device,
		&_deviceSupport,
		_memorySystem,
		_descriptorSetLayout,
		_transferCommandPool,
		_graphicsQueue);

	CreateSwapchain();
}

Video::~Video()
{
	DestroySwapchain();

	RemoveAllModels();
	DestroySkybox();

	delete _scene.Textures;
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
	descriptor.Textures = model->GetTextures();

	return descriptor;
}

void Video::DestroyModelDescriptor(ModelDescriptor descriptor)
{
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
	descriptor.Textures = rectangle->GetTextures();

	return descriptor;
}

void Video::DestroyRectangleDescriptor(ModelDescriptor descriptor)
{
}

void Video::CreateSkybox(
	uint32_t texWidth,
	uint32_t texHeight,
	const std::vector<uint8_t>& texData)
{
	DestroySkybox();

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

	uint32_t skyboxTexture = _scene.Textures->AddTexture(
		texWidth,
		texHeight,
		texDataTransformed,
		TextureHandler::TextureType::TCube,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		layerCount);
	_scene.skybox.Descriptor.Textures = {skyboxTexture};

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

	_scene.Textures->RemoveTexture(_scene.skybox.Descriptor.Textures[0]);
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
