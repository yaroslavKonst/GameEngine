#ifndef _IMAGE_HELPER_H
#define _IMAGE_HELPER_H

#include "MemorySystem.h"
#include "PhysicalDeviceSupport.h"
#include "CommandPool.h"
#include "BufferHelper.h"
#include "VkQueueObject.h"

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
		PhysicalDeviceSupport* deviceSupport,
		VkImageCreateFlagBits flags = (VkImageCreateFlagBits)0,
		uint32_t layerCount = 1);

	void DestroyImage(
		VkDevice device,
		Image image,
		MemorySystem* memorySystem);

	VkImageView CreateImageView(
		VkDevice device,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags,
		uint32_t mipLevels,
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
		uint32_t layerCount = 1);

	void DestroyImageView(
		VkDevice device,
		VkImageView imageView);

	void ChangeImageLayout(
		Image image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		uint32_t mipLevels,
		CommandPool* commandPool,
		VkQueueObject* graphicsQueue,
		uint32_t layerCount = 1);

	void CopyBufferToImage(
		BufferHelper::Buffer buffer,
		Image image,
		uint32_t width,
		uint32_t height,
		CommandPool* commandPool,
		VkQueueObject* graphicsQueue,
		uint32_t layerCount = 1);

	void RotateClockWise(
		uint8_t* image,
		uint32_t width,
		uint32_t height);

	void RotateCounterClockWise(
		uint8_t* image,
		uint32_t width,
		uint32_t height);

	void FlipVertically(
		uint8_t* image,
		uint32_t width,
		uint32_t height);

	void FlipHorizontally(
		uint8_t* image,
		uint32_t width,
		uint32_t height);

	void Swap(
		uint8_t* image1,
		uint8_t* image2,
		uint32_t width,
		uint32_t height);

	VkSampler CreateImageSampler(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		float mipLevels,
		VkSamplerAddressMode modeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerAddressMode modeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerAddressMode modeW = VK_SAMPLER_ADDRESS_MODE_REPEAT);
	void DestroyImageSampler(VkDevice device, VkSampler sampler);
}

#endif
