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
#include "ModelDescriptor.h"
#include "BufferHelper.h"
#include "InputControl.h"
#include "SceneDescriptor.h"

class Video
{
public:
	struct GraphicsSettings
	{
		uint32_t MsaaLimit;
	};

	Video(
		int width,
		int height,
		std::string name,
		std::string applicationName = "",
		GraphicsSettings* settings = nullptr);

	~Video();

	Video(const Video& video) = delete;
	Video& operator=(const Video& video) = delete;

	void MainLoop();
	void Stop();

	void RegisterModel(Model* model);
	void RemoveModel(Model* model);

	void RegisterRectangle(Rectangle* rectangle);
	void RemoveRectangle(Rectangle* rectangle);

	void RegisterLight(Light* light);
	void RemoveLight(Light* light);

	void SetFOV(double fov)
	{
		_scene.FOV = fov;
	}

	void SetCameraPosition(const glm::vec3& value)
	{
		_scene.CameraPosition = value;
	}

	void SetCameraDirection(const glm::vec3& value)
	{
		_scene.CameraDirection = value;
	}

	void SetCameraUp(const glm::vec3& value)
	{
		_scene.CameraUp = value;
	}

	void SetCameraTarget(const glm::vec3& value)
	{
		_scene.CameraDirection = value - _scene.CameraPosition;
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
	void SetSkyboxColor(glm::vec3 color)
	{
		_scene.skybox.ColorModifier = color;
	}

	void SetSceneMutex(std::mutex* mutex)
	{
		_scene.SceneMutex = mutex;
	}

	float GetScreenRatio()
	{
		return _swapchain->GetScreenRatio();
	}

	TextureHandler* GetTextures()
	{
		return _scene.Textures;
	}

private:
	Window _window;

	bool _settingsValid;
	GraphicsSettings _settings;

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

	SceneDescriptor _scene;
	ModelDescriptor CreateModelDescriptor(Model* model);
	void DestroyModelDescriptor(ModelDescriptor descriptor);
	void RemoveAllModels();

	ModelDescriptor CreateRectangleDescriptor(Rectangle* rectangle);
	void DestroyRectangleDescriptor(ModelDescriptor descriptor);
	void RemoveAllRectangles();

	VkDescriptorSetLayout _descriptorSetLayout;
	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();

	InputControl* _inputControl;
};

#endif
