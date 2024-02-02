#ifndef _PHYSICAL_DEVICE_SUPPORT
#define _PHYSICAL_DEVICE_SUPPORT

#include <optional>
#include <vector>
#include <stdexcept>
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

	uint32_t FindMemoryType(
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties);

	VkFormat FindSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);

	VkPhysicalDevice GetPhysicalDevice()
	{
		return _device;
	}

private:
	VkPhysicalDevice _device;
	VkSurfaceKHR _surface;
};

#endif
