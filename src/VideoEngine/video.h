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

	uint32_t CreateSkyboxTexture(
		const Loader::Image& image,
		bool async = false);
	void DestroySkyboxTexture(uint32_t texture);

	void SetSkyboxNumber(uint32_t count)
	{
		_dataBridge.StagedScene.skybox.resize(count);

		for (size_t idx = 0; idx < count; ++idx) {
			_dataBridge.StagedScene.skybox[idx] = Skybox();
		}
	}

	void SetSkyboxEnabled(bool enabled, uint32_t index = 0)
	{
		_dataBridge.StagedScene.skybox[index].Enabled = enabled;
	}

	void SetSkyboxTexture(uint32_t texture, uint32_t index = 0)
	{
		_dataBridge.StagedScene.skybox[index].Texture = texture;
	}

	void SetSkyboxColor(const glm::vec3& color, uint32_t index = 0)
	{
		_dataBridge.StagedScene.skybox[index].ColorMultiplier = color;
	}

	void SetSkyboxGradient(
		bool enabled,
		const glm::vec3& value,
		float offset,
		uint32_t index = 0)
	{
		_dataBridge.StagedScene.skybox[index].GradientEnabled = enabled;
		_dataBridge.StagedScene.skybox[index].GradientOffset = offset;
		_dataBridge.StagedScene.skybox[index].Gradient = value;
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
