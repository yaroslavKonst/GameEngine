#include "VkInstanceHandler.h"

#include <GLFW/glfw3.h>

namespace VkInstanceHandler
{
	static int _refCounter = 0;
	static VkInstance _instance;
	static std::string _appName;

	void CreateInstance()
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = _appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions =
			glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		instanceInfo.enabledExtensionCount = glfwExtensionCount;
		instanceInfo.ppEnabledExtensionNames = glfwExtensions;

		instanceInfo.enabledLayerCount = 0;

		VkResult res = vkCreateInstance(
			&instanceInfo,
			nullptr,
			&_instance);

		if (res != VK_SUCCESS) {
			throw std::runtime_error(
				"Failed to create VkInstance.");
		}
	}

	VkInstance& GetInstance()
	{
		return _instance;
	}

	void IncRef()
	{
		if (_refCounter == 0) {
			CreateInstance();
		}

		++_refCounter;
	}

	void DecRef()
	{
		--_refCounter;

		if (_refCounter == 0) {
			vkDestroyInstance(_instance, nullptr);
		}
	}

	void SetApplicationName(std::string name)
	{
		_appName = name;
	}
}
