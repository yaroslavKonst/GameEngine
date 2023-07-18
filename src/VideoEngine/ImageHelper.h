#ifndef _IMAGE_HELPER_H
#define _IMAGE_HELPER_H

#include "MemorySystem.h"
#include "PhysicalDeviceSupport.h"

namespace ImageHelper
{
	struct Image
	{
		VkImage Image;
		VkFormat Format;
		MemorySystem::Allocation Allocation;
	};

	Image CreateImage(
		VkDevice device,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkSampleCountFlagBits numSamples,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		MemorySystem* memorySystem,
		PhysicalDeviceSupport* deviceSupport);

	void DestroyImage(
		VkDevice device,
		Image image,
		MemorySystem* memorySystem);

	VkImageView CreateImageView(
		VkDevice device,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags,
		uint32_t mipLevels);
	void DestroyImageView(
		VkDevice device,
		VkImageView imageView);
}

#endif
