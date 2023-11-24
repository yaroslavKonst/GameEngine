#ifndef _VIDEO_H
#define _VIDEO_H

#include <vector>
#include <map>
#include <optional>

#include "window.h"
#include "VkInstanceHandler.h"
#include "VkQueueObject.h"
#include "CommandPool.h"
#include "swapchain.h"
#include "PhysicalDeviceSupport.h"
#include "MemorySystem.h"
#include "ModelDescriptor.h"
#include "BufferHelper.h"
#include "DataBridge.h"
#include "../Utils/ThreadPool.h"

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

	uint32_t LoadModel(Loader::VertexData model, bool async = false);
	void UnloadModel(uint32_t model);

	void RegisterModel(Model* model);
	void RemoveModel(Model* model);

	void RegisterRectangle(Rectangle* rectangle);
	void RemoveRectangle(Rectangle* rectangle);

	void RegisterLight(Light* light);
	void RemoveLight(Light* light);

	void RegisterSprite(Sprite* sprite);
	void RemoveSprite(Sprite* sprite);

	void SubmitScene()
	{
		_dataBridge.Submit();
	}

	void SetFOV(double fov)
	{
		_dataBridge.StagedScene.FOV = fov;
	}

	void SetCameraPosition(const glm::vec3& value)
	{
		_dataBridge.StagedScene.CameraPosition = value;
	}

	void SetCameraDirection(const glm::vec3& value)
	{
		_dataBridge.StagedScene.CameraDirection = value;
	}

	void SetCameraUp(const glm::vec3& value)
	{
		_dataBridge.StagedScene.CameraUp = value;
	}

	void SetCameraTarget(const glm::vec3& value)
	{
		_dataBridge.StagedScene.CameraDirection =
			value - _dataBridge.StagedScene.CameraPosition;
	}

	InputControl* GetInputControl()
	{
		return _dataBridge.inputControl;
	}

	void CreateSkybox(
		uint32_t texWidth,
		uint32_t texHeight,
		const std::vector<uint8_t>& texData);
	void DestroySkybox();
	void SetSkyboxColor(glm::vec3 color)
	{
		_dataBridge.skybox.SetColorMultiplier(glm::vec4(color, 1.0));
	}

	float GetScreenRatio()
	{
		return _swapchain->GetScreenRatio();
	}

	TextureHandler* GetTextures()
	{
		return _dataBridge.Textures;
	}

private:
	Window _window;

	bool _settingsValid;
	GraphicsSettings _settings;

	ThreadPool* _loaderThreadPool;

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
	VkQueueObject _graphicsQueue;
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

	DataBridge _dataBridge;
	void RemoveAllModels();
	std::mutex _modelLoadMutex;

	VkDescriptorSetLayout _descriptorSetLayout;
	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();
};

#endif
