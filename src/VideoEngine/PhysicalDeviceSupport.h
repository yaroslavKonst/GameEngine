#ifndef _PHYSICAL_DEVICE_SUPPORT
#define _PHYSICAL_DEVICE_SUPPORT

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

class PhysicalDeviceSupport
{
public:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	void SetPhysicalDevice(VkPhysicalDevice device);
	void SetSurface(VkSurfaceKHR surface);

	SwapchainSupportDetails QuerySwapchainSupport();
	QueueFamilyIndices FindQueueFamilies();

private:
	VkPhysicalDevice _device;
	VkSurfaceKHR _surface;
};

#endif
