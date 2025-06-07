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
	if (settings) {
		_settings = *settings;
		_settingsValid = true;
	} else {
		_settingsValid = false;
	}

	LoadScaling();

	_loaderThreadPool = new ThreadPool(1);

	VkInstanceHandler::SetApplicationName(applicationName);
	VkInstanceHandler::IncRef();

	_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	CreateSurface();
	SelectPhysicalDevice();
	CreateDevice();
	_dataBridge.inputControl = new InputControl(_window.GetWindow());
	CreateCommandPools();
	CreateDescriptorSetLayout();
	_dataBridge.Textures = new TextureHandler(
		_device,
		&_deviceSupport,
		_memorySystem,
		_descriptorSetLayout,
		_transferCommandPool,
		&_graphicsQueue,
		_loaderThreadPool);

	_dataBridge.UniformBuffers = new UniformBufferStorage(
		_device,
		_memorySystem,
		&_deviceSupport);

	CreateSwapchain();

	_resizeHandler = new ResizeHandler(_swapchain);
	_dataBridge.inputControl->Subscribe(_resizeHandler);
}

Video::~Video()
{
	delete _loaderThreadPool;

	_dataBridge.inputControl->Unsubscribe(_resizeHandler);
	delete _resizeHandler;

	DestroySwapchain();

	RemoveAllModels();

	delete _dataBridge.UniformBuffers;
	delete _dataBridge.Textures;
	DestroyDescriptorSetLayout();
	DestroyCommandPools();
	delete _dataBridge.inputControl;
	DestroyDevice();
	DestroySurface();

	VkInstanceHandler::DecRef();
}

void Video::LoadScaling()
{
	if (!_settingsValid) {
		_scaling = 1;
	} else {
		_scaling = _settings.Scaling;
	}

	Logger::Verbose() << "Scaling: " << _scaling;
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

	bool discreteGpu = deviceProperties.deviceType ==
		VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	if (!discreteGpu) {
		Logger::Error() << "Not a discrete GPU.";
	}

	if (!extensionsSupported) {
		Logger::Error() << "Unsupported extensions.";
	}

	if (!swapchainAdequate) {
		Logger::Error() << "Unsupported swapchain.";
	}

	if (!deviceFeatures.geometryShader) {
		Logger::Error() << "Geometry shaders are not supported.";
	}

	if (!deviceFeatures.samplerAnisotropy) {
		Logger::Error() << "Anisotropic filtering is not supported.";
	}

	if (!deviceFeatures.shaderUniformBufferArrayDynamicIndexing) {
		Logger::Error() <<
			"Dynamic indexing of UBO is not supported.";
	}

	if (!_deviceSupport.FindQueueFamilies().graphicsFamily.has_value()) {
		Logger::Error() << "Graphics queue is not available.";
	}

	if (!_deviceSupport.FindQueueFamilies().presentFamily.has_value()) {
		Logger::Error() << "Present queue is not available.";
	}

	bool res =
		discreteGpu &&
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
		&_graphicsQueue.Queue);

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
		_scaling,
		&_graphicsQueue,
		_presentQueue,
		&_dataBridge,
		_descriptorSetLayout,
		10,
		1024);

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

void Video::UnloadModel(uint32_t model)
{
	_dataBridge.RemoveModelMessages.Insert({model});

	_modelLoadMutex.Lock();
	_dataBridge.UsedModelDescriptors.erase(model);
	_modelLoadMutex.Unlock();
}

uint32_t Video::LoadModel(const Loader::VertexData& model, bool async)
{
	_modelLoadMutex.Lock();
	uint32_t index = _dataBridge.LastModelIndex + 1;

	while (_dataBridge.UsedModelDescriptors.find(index) !=
		_dataBridge.UsedModelDescriptors.end())
	{
		++index;
	}

	_dataBridge.LastModelIndex = index;
	_dataBridge.UsedModelDescriptors.insert(index);
	_modelLoadMutex.Unlock();

	Loader::VertexData* vertexData = new Loader::VertexData();
	*vertexData = model;

	uint32_t id = _loaderThreadPool->Enqueue(
		[this, vertexData, index]() -> void
		{
			auto descriptor =
				ModelDescriptor::CreateModelDescriptor(
					vertexData,
					_device,
					_memorySystem,
					&_deviceSupport,
					&_graphicsQueue,
					_transferCommandPool);
				delete vertexData;
			_dataBridge.LoadModelMessages.Insert(
				{index, descriptor});
		},
		!async);

	if (!async) {
		_loaderThreadPool->Wait(id);
	}

	return index;
}

void Video::RegisterModel(Model* model)
{
	_dataBridge.ExtModMutex.Lock();
	_dataBridge.StagedScene.Models.insert(model);
	_dataBridge.ExtModMutex.Unlock();
}

void Video::RemoveModel(Model* model)
{
	_dataBridge.ExtModMutex.Lock();
	_dataBridge.StagedScene.Models.erase(model);
	_dataBridge.StagedScene.RemovedModels.insert(model);
	_dataBridge.ExtModMutex.Unlock();
}

void Video::RemoveAllModels()
{
	vkQueueWaitIdle(_graphicsQueue.Queue);

	for (auto& model : _dataBridge.ModelDescriptors) {
		ModelDescriptor::DestroyModelDescriptor(
			model.second,
			_device,
			_memorySystem);
	}

	_dataBridge.StagedScene.Models.clear();

	for (auto& desc : _dataBridge.DeletedModelDescriptors) {
		ModelDescriptor::DestroyModelDescriptor(
			desc,
			_device,
			_memorySystem);
	}

	_dataBridge.DeletedModelDescriptors.clear();
}

void Video::RegisterRectangle(Rectangle* rectangle)
{
	_dataBridge.ExtModMutex.Lock();
	_dataBridge.StagedScene.Rectangles.insert(rectangle);
	_dataBridge.ExtModMutex.Unlock();
}

void Video::RemoveRectangle(Rectangle* rectangle)
{
	_dataBridge.ExtModMutex.Lock();
	_dataBridge.StagedScene.Rectangles.erase(rectangle);
	_dataBridge.ExtModMutex.Unlock();
}

uint32_t Video::LoadSkyboxTexture(
	const Loader::Image& image,
	bool async)
{
	uint32_t texWidth = image.Width;
	uint32_t texHeight = image.Height;
	uint32_t layerCount = 6;

	// Texture layout transformation.
	texWidth /= layerCount;

	Loader::Image resImage;
	resImage.Width = texWidth;
	resImage.Height = texHeight;
	resImage.PixelData.resize(image.PixelData.size(), 100);

	for (uint32_t layer = 0; layer < layerCount; ++layer) {
		for (uint32_t y = 0; y < texHeight; ++y) {
			for (uint32_t x = 0; x < texWidth; ++x) {
				for (uint32_t c = 0; c < 4; ++c) {
					resImage.PixelData[
						(texWidth * texHeight * layer +
						texWidth * y +
						x) * 4 + c] =
					image.PixelData[(texWidth * layer +
						x +
						texWidth * layerCount * y) *
						4 + c];
				}
			}
		}
	}

	// 1.
	ImageHelper::RotateClockWise(
		resImage.PixelData.data(),
		texWidth,
		texHeight);
	ImageHelper::FlipHorizontally(
		resImage.PixelData.data(),
		texHeight,
		texWidth);

	// 2.
	ImageHelper::FlipHorizontally(
		resImage.PixelData.data() + texWidth * texHeight * 4,
		texHeight,
		texWidth);

	// 3.
	ImageHelper::RotateCounterClockWise(
		resImage.PixelData.data() + texWidth * texHeight * 4 * 2,
		texWidth,
		texHeight);
	ImageHelper::FlipHorizontally(
		resImage.PixelData.data() + texWidth * texHeight * 4 * 2,
		texHeight,
		texWidth);

	// 4.
	ImageHelper::FlipVertically(
		resImage.PixelData.data() + texWidth * texHeight * 4 * 3,
		texHeight,
		texWidth);

	// Ordering.
	ImageHelper::Swap(
		resImage.PixelData.data() + texWidth * texHeight * 4 * 2,
		resImage.PixelData.data() + texWidth * texHeight * 4,
		texWidth,
		texHeight);
	ImageHelper::Swap(
		resImage.PixelData.data() + texWidth * texHeight * 4 * 3,
		resImage.PixelData.data() + texWidth * texHeight * 4 * 2,
		texWidth,
		texHeight);
	ImageHelper::Swap(
		resImage.PixelData.data() + texWidth * texHeight * 4 * 4,
		resImage.PixelData.data() + texWidth * texHeight * 4 * 5,
		texWidth,
		texHeight);

	return _dataBridge.Textures->AddTexture(
		resImage,
		true,
		async,
		TextureHandler::TextureType::TCube,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		layerCount);
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
	_dataBridge.ExtModMutex.Lock();
	_dataBridge.StagedScene.Lights.insert(light);
	_dataBridge.ExtModMutex.Unlock();
}

void Video::RemoveLight(Light* light)
{
	_dataBridge.ExtModMutex.Lock();
	_dataBridge.StagedScene.Lights.erase(light);
	_dataBridge.ExtModMutex.Unlock();
}

void Video::RegisterSprite(Sprite* sprite)
{
	_dataBridge.ExtModMutex.Lock();
	_dataBridge.StagedScene.Sprites.insert(sprite);
	_dataBridge.ExtModMutex.Unlock();
}

void Video::RemoveSprite(Sprite* sprite)
{
	_dataBridge.ExtModMutex.Lock();
	_dataBridge.StagedScene.Sprites.erase(sprite);
	_dataBridge.ExtModMutex.Unlock();
}
