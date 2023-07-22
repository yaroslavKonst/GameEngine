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
#include "skybox.h"

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

	void SetCameraPosition(const glm::vec3& value)
	{
		_cameraPosition = value;
	}

	void SetCameraDirection(const glm::vec3& value)
	{
		_cameraDirection = value;
	}

	void SetCameraUp(const glm::vec3& value)
	{
		_cameraUp = value;
	}

	void SetCameraTarget(const glm::vec3& value)
	{
		_cameraDirection = value - _cameraPosition;
	}

	InputControl* GetInputControl()
	{
		return _inputControl;
	}

	void CreateSkybox(
		uint32_t texWidth,
		uint32_t texHeight,
		const std::vector<uint8_t>& texData);
	void DestroySkybox();

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

	Skybox _skybox;

	ImageHelper::Image CreateTextureImage(
		Texturable* model,
		uint32_t& mipLevels,
		VkImageCreateFlagBits flags = (VkImageCreateFlagBits)0,
		uint32_t layerCount = 1);
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
	glm::vec3 _cameraPosition;
	glm::vec3 _cameraDirection;
	glm::vec3 _cameraUp;

	InputControl* _inputControl;
};

#endif
