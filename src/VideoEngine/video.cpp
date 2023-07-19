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

	CreateSwapchain();
}

Video::~Video()
{
	DestroySwapchain();

	RemoveAllModels();
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
		&_models);

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
}

void Video::RemoveModel(Model* model)
{
	DestroyModelDescriptor(_models[model]);
	_models.erase(model);
}

void Video::RemoveAllModels()
{
	for (auto& model : _models) {
		DestroyModelDescriptor(model.second);
	}

	_models.clear();
}

ModelDescriptor Video::CreateModelDescriptor(Model* model)
{
	ModelDescriptor descriptor;

	auto& vertices = model->GetModelVertices();
	auto& colors = model->GetModelColors();

	descriptor.VertexBuffer = BufferHelper::CreateBuffer(
		_device,
		sizeof(ModelDescriptor::Vertex) * vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		_memorySystem,
		&_deviceSupport);

	ModelDescriptor::Vertex* data;
	vkMapMemory(
		_device,
		descriptor.VertexBuffer.Allocation.Memory,
		descriptor.VertexBuffer.Allocation.Offset,
		descriptor.VertexBuffer.Allocation.Size,
		0,
		reinterpret_cast<void**>(&data));

	for (size_t i = 0; i < vertices.size(); ++i) {
		data[i].pos = vertices[i];
		data[i].color = colors[i];
	}

	vkUnmapMemory(
		_device,
		descriptor.VertexBuffer.Allocation.Memory);

	descriptor.VertexCount = vertices.size();

	return descriptor;
}

void Video::DestroyModelDescriptor(ModelDescriptor descriptor)
{
	BufferHelper::DestroyBuffer(
		_device,
		descriptor.VertexBuffer,
		_memorySystem);
}
