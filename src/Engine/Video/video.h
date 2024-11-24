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
		double Scaling;
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

	uint32_t LoadModel(const Loader::VertexData& model, bool async = false);
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

	void SetCameraPosition(const Math::Vec<3>& value)
	{
		_dataBridge.StagedScene.CameraPosition = value;
	}

	void SetCameraDirection(const Math::Vec<3>& value)
	{
		_dataBridge.StagedScene.CameraDirection = value;
	}

	void SetCameraUp(const Math::Vec<3>& value)
	{
		_dataBridge.StagedScene.CameraUp = value;
	}

	void SetCameraTarget(const Math::Vec<3>& value)
	{
		_dataBridge.StagedScene.CameraDirection =
			value - _dataBridge.StagedScene.CameraPosition;
	}

	void Subscribe(InputHandler* handler)
	{
		_dataBridge.inputControl->Subscribe(handler);
	}

	void Unsubscribe(InputHandler* handler)
	{
		_dataBridge.inputControl->Unsubscribe(handler);
	}

	void ToggleRawMouseInput()
	{
		_dataBridge.inputControl->ToggleRawMouseInput();
	}

	uint32_t LoadTexture(
		const Loader::Image& image,
		bool repeat = true,
		bool async = false)
	{
		return _dataBridge.Textures->AddTexture(image, repeat, async);
	}

	void UnloadTexture(uint32_t index)
	{
		_dataBridge.Textures->RemoveTexture(index);
	}

	uint32_t LoadSkyboxTexture(
		const Loader::Image& image,
		bool async = false);

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
	double _scaling;
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
	Sync::Mutex _modelLoadMutex;

	VkDescriptorSetLayout _descriptorSetLayout;
	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();

	void LoadScaling();

	class ResizeHandler : public InputHandler
	{
	public:
		ResizeHandler(Swapchain* swapchain)
		{
			_swapchain = swapchain;
			SetInputLayer(10000);
			SetInputEnabled(true);
		}

		void WindowResize() override
		{
			_swapchain->RequestReload();
		}

	private:
		Swapchain* _swapchain;
	};

	ResizeHandler* _resizeHandler;
};

#endif
