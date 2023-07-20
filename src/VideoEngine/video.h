#ifndef _VIDEO_H
#define _VIDEO_H

#include <vector>
#include <map>
#include <optional>

#include "window.h"
#include "VkInstanceHandler.h"
#include "CommandPool.h"
#include "swapchain.h"
#include "PhysicalDeviceSupport.h"
#include "MemorySystem.h"
#include "model.h"
#include "rectangle.h"
#include "ModelDescriptor.h"
#include "BufferHelper.h"
#include "InputControl.h"

class Video
{
public:
	Video(
		int width,
		int height,
		std::string name,
		std::string applicationName = "");

	~Video();

	Video(const Video& video) = delete;
	Video& operator=(const Video& video) = delete;

	void MainLoop();
	void Stop();

	void RegisterModel(Model* model);
	void RemoveModel(Model* model);

	void RegisterRectangle(Rectangle* rectangle);
	void RemoveRectangle(Rectangle* rectangle);

	void SetFOV(double fov)
	{
		_fov = fov;
	}

	void SetViewMatrix(const glm::mat4& matrix)
	{
		_viewMatrix = matrix;
	}

	InputControl* GetInputControl()
	{
		return _inputControl;
	}

private:
	Window _window;

	VkSurfaceKHR _surface;
	void CreateSurface();
	void DestroySurface();

	VkPhysicalDevice _physicalDevice;
	VkSampleCountFlagBits _msaaSamples;
	std::vector<const char*> _deviceExtensions;
	void SelectPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	VkSampleCountFlagBits GetMaxSampleCount();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	PhysicalDeviceSupport _deviceSupport;

	VkDevice _device;
	VkQueue _graphicsQueue;
	VkQueue _presentQueue;
	MemorySystem* _memorySystem;
	void CreateDevice();
	void DestroyDevice();

	CommandPool* _transferCommandPool;
	void CreateCommandPools();
	void DestroyCommandPools();

	Swapchain* _swapchain;
	void CreateSwapchain();
	void DestroySwapchain();

	std::map<Model*, ModelDescriptor> _models;
	ModelDescriptor CreateModelDescriptor(Model* model);
	void DestroyModelDescriptor(ModelDescriptor descriptor);
	void RemoveAllModels();

	std::map<Rectangle*, ModelDescriptor> _rectangles;
	ModelDescriptor CreateRectangleDescriptor(Rectangle* rectangle);
	void DestroyRectangleDescriptor(ModelDescriptor descriptor);
	void RemoveAllRectangles();

	ImageHelper::Image CreateTextureImage(
		Texturable* model,
		uint32_t& mipLevels);
	VkSampler CreateTextureSampler(float mipLevels);
	void DestroyTextureSampler(VkSampler sampler);
	void CreateDescriptorSets(ModelDescriptor* descriptor);
	void DestroyDescriptorSets(ModelDescriptor* descriptor);
	void GenerateMipmaps(
		ImageHelper::Image image,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels);

	VkDescriptorSetLayout _descriptorSetLayout;
	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();

	double _fov;
	glm::mat4 _viewMatrix;

	InputControl* _inputControl;
};

#endif
