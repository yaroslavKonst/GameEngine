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
#include "ModelDescriptor.h"
#include "BufferHelper.h"

class Video
{
public:
	Video(
		int width,
		int height,
		std::string name,
		std::string applicationName = "");

	~Video();

	void MainLoop();
	void Stop();

	void RegisterModel(Model* model);
	void RemoveModel(Model* model);

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
	void RemoveModelDescriptor(ModelDescriptor modelDescriptor);
	void RemoveAllModels();
};

#endif
