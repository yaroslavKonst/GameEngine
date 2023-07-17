#include "PhysicalDeviceSupport.h"

namespace PhysicalDeviceSupport
{
	static VkPhysicalDevice _device;
	static VkSurfaceKHR _surface;

	void SetPhysicalDevice(VkPhysicalDevice device)
	{
		_device = device;
	}

	void SetSurface(VkSurfaceKHR surface)
	{
		_surface = surface;
	}

	SwapchainSupportDetails QuerySwapchainSupport()
	{
		SwapchainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			_device,
			_surface,
			&details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			_device,
			_surface,
			&formatCount,
			nullptr);

		if (formatCount > 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				_device,
				_surface,
				&formatCount,
				details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			_device,
			_surface,
			&presentModeCount,
			nullptr);

		if (presentModeCount > 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			_device,
			_surface,
			&presentModeCount,
			details.presentModes.data());
		}

		return details;
	}

	QueueFamilyIndices FindQueueFamilies()
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(
			_device,
			&queueFamilyCount,
			nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(
			queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
			_device,
			&queueFamilyCount,
			queueFamilies.data());

		int i = 0;

		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(
				_device,
				i,
				_surface,
				&presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			++i;
		}

		return indices;
	}
}
