#include "swapchain.h"

#include <algorithm>

#include "../Logger/logger.h"
#include "mvp.h"

Swapchain::Swapchain(
	VkDevice device,
	VkSurfaceKHR surface,
	GLFWwindow* window,
	PhysicalDeviceSupport* deviceSupport,
	MemorySystem* memorySystem,
	VkSampleCountFlagBits msaaSamples,
	VkQueue graphicsQueue,
	VkQueue presentQueue,
	std::map<Model*, ModelDescriptor>* models,
	glm::mat4* viewMatrix,
	double* fov,
	VkDescriptorSetLayout descriptorSetLayout)
{
	_device = device;
	_surface = surface;
	_window = window;
	_deviceSupport = deviceSupport;
	_memorySystem = memorySystem;
	_msaaSamples = msaaSamples;
	_graphicsQueue = graphicsQueue;
	_presentQueue = presentQueue;
	_models = models;
	_viewMatrix = viewMatrix;
	_fov = fov;
	_descriptorSetLayout = descriptorSetLayout;

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

	_commandPool = new CommandPool(
		_device,
		indices.graphicsFamily.value(),
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	_commandBuffers.resize(_maxFramesInFlight);

	for (size_t i = 0; i < _maxFramesInFlight; ++i) {
		_commandBuffers[i] = _commandPool->CreateCommandBuffer();
	}

	CreateSyncObjects();

	_currentFrame = 0;
	_initialized = true;
}

void Swapchain::Destroy()
{
	DestroySyncObjects();
	delete _commandPool;
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
		_imageFormat,
		_descriptorSetLayout);

	_pipeline->CreateFramebuffers(_imageViews);
}

void Swapchain::DestroyPipelines()
{
	_pipeline->DestroyFramebuffers();
	delete _pipeline;
}

void Swapchain::RecordCommandBuffer(
	VkCommandBuffer commandBuffer,
	uint32_t imageIndex)
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

	_pipeline->RecordCommandBuffer(commandBuffer, imageIndex);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_extent.width);
	viewport.height = static_cast<float>(_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = _extent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	MVP mvp;
	mvp.View = *_viewMatrix;
	mvp.Proj = glm::perspective(
		glm::radians((float)*_fov),
		(float)_extent.width / (float)_extent.height,
		0.1f,
		10.0f);

	mvp.Proj[1][1] *= -1;

	for (auto& model : *_models) {
		if (!model.first->IsModelActive()) {
			continue;
		}

		mvp.Model = model.first->GetModelMatrix();

		VkBuffer vertexBuffers[] = {model.second.VertexBuffer.Buffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(
			commandBuffer,
			0,
			1,
			vertexBuffers,
			offsets);

		vkCmdBindIndexBuffer(
			commandBuffer,
			model.second.IndexBuffer.Buffer,
			0,
			VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(
			commandBuffer,
			_pipeline->GetPipelineLayout(),
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(MVP),
			&mvp);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			_pipeline->GetPipelineLayout(),
			0,
			1,
			&model.second.DescriptorSet,
			0,
			nullptr);

		vkCmdDrawIndexed(
			commandBuffer,
			model.second.IndexCount,
			1,
			0,
			0,
			0);
	}

	vkCmdEndRenderPass(commandBuffer);

	res = vkEndCommandBuffer(commandBuffer);
	
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to end command buffer.");
	}
}

void Swapchain::MainLoop() {
	_work = true;

	while (!glfwWindowShouldClose(_window) && _work) {
		glfwPollEvents();
		DrawFrame();
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

	RecordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

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

	res = vkQueueSubmit(
		_graphicsQueue,
		1,
		&submitInfo,
		_inFlightFences[_currentFrame]);
	
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
