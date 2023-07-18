#include "ImageHelper.h"

namespace ImageHelper
{
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
		PhysicalDeviceSupport* deviceSupport)
	{
		Image image;
		image.Format = format;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = 0;

		VkResult res = vkCreateImage(
			device,
			&imageInfo,
			nullptr,
			&image.Image);

		if (res != VK_SUCCESS) {
			throw std::runtime_error("failed to create image");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(
			device,
			image.Image,
			&memRequirements);

		MemorySystem::AllocationProperties allocProps;
		allocProps.MemoryTypeIndex = deviceSupport->FindMemoryType(
			memRequirements.memoryTypeBits,
			properties);
		allocProps.Alignment = memRequirements.alignment;

		image.Allocation = memorySystem->Allocate(
			memRequirements.size,
			allocProps);

		vkBindImageMemory(
			device,
			image.Image,
			image.Allocation.Memory,
			image.Allocation.Offset);

		return image;
	}

	void DestroyImage(
		VkDevice device,
		Image image,
		MemorySystem* memorySystem)
	{
		vkDestroyImage(device, image.Image, nullptr);
		memorySystem->Free(image.Allocation);
	}
}
