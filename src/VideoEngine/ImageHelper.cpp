#include "ImageHelper.h"

#include <cstring>

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
		PhysicalDeviceSupport* deviceSupport,
		VkImageCreateFlagBits flags,
		uint32_t layerCount)
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
		imageInfo.arrayLayers = layerCount;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = flags;

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
		allocProps.Mapped = false;

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

	VkImageView CreateImageView(
		VkDevice device,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags,
		uint32_t mipLevels,
		VkImageViewType viewType,
		uint32_t layerCount)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = viewType;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = layerCount;

		VkImageView imageView;

		VkResult res;
		res = vkCreateImageView(device, &viewInfo, nullptr, &imageView);

		if (res != VK_SUCCESS) {
			throw std::runtime_error(
				"Failed to create image view.");
		}

		return imageView;
	}

	void DestroyImageView(
		VkDevice device,
		VkImageView imageView)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	bool HasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
			format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void ChangeImageLayout(
		Image image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		uint32_t mipLevels,
		CommandPool* commandPool,
		VkQueueObject* graphicsQueue,
		uint32_t layerCount)
	{
		VkCommandBuffer commandBuffer =
			commandPool->StartOneTimeBuffer();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image.Image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;

		if (newLayout ==
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask =
				VK_IMAGE_ASPECT_DEPTH_BIT;

			if (HasStencilComponent(image.Format)) {
				barrier.subresourceRange.aspectMask |=
					VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		} else {
			barrier.subresourceRange.aspectMask =
				VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (
			oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (
			oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage =
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else if (
			oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			newLayout ==
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage =
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		} else {
			throw std::invalid_argument(
				"Unsupported layout transition.");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		commandPool->EndOneTimeBuffer(commandBuffer, graphicsQueue);
	}

	void CopyBufferToImage(
		BufferHelper::Buffer buffer,
		Image image,
		uint32_t width,
		uint32_t height,
		CommandPool* commandPool,
		VkQueueObject* graphicsQueue,
		uint32_t layerCount)
	{
		VkCommandBuffer commandBuffer =
			commandPool->StartOneTimeBuffer();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer.Buffer,
			image.Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);

		commandPool->EndOneTimeBuffer(commandBuffer, graphicsQueue);
	}

	void RotateClockWise(
		uint8_t* image,
		uint32_t width,
		uint32_t height)
	{
		std::vector<uint8_t> buffer(width * height * 4);

		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				for (uint32_t c = 0; c < 4; ++c) {
					buffer[(height * x + height - 1 - y) *
						4 + c] =
					image[(width * y + x) * 4 + c];
				}
			}
		}

		memcpy(image, buffer.data(), width * height * 4);
	}

	void RotateCounterClockWise(
		uint8_t* image,
		uint32_t width,
		uint32_t height)
	{
		std::vector<uint8_t> buffer(width * height * 4);

		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				for (uint32_t c = 0; c < 4; ++c) {
					buffer[(height * (width - 1 - x) + y) *
						4 + c] =
					image[(width * y + x) * 4 + c];
				}
			}
		}

		memcpy(image, buffer.data(), width * height * 4);
	}

	void FlipVertically(
		uint8_t* image,
		uint32_t width,
		uint32_t height)
	{
		for (uint32_t y = 0; y < height / 2; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				for (uint32_t c = 0; c < 4; ++c) {
					uint32_t upIndex = (width * y + x) *
						4 + c;
					uint32_t downIndex =
						(width * (height - 1 - y) + x) *
						4 + c;

					uint8_t tmp = image[downIndex];
					image[downIndex] = image[upIndex];
					image[upIndex] = tmp;
				}
			}
		}
	}

	void FlipHorizontally(
		uint8_t* image,
		uint32_t width,
		uint32_t height)
	{
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width / 2; ++x) {
				for (uint32_t c = 0; c < 4; ++c) {
					uint32_t leftIndex = (width * y + x) *
						4 + c;
					uint32_t rightIndex =
						(width * y + width - 1 - x) *
						4 + c;

					uint8_t tmp = image[rightIndex];
					image[rightIndex] = image[leftIndex];
					image[leftIndex] = tmp;
				}
			}
		}
	}

	void Swap(
		uint8_t* image1,
		uint8_t* image2,
		uint32_t width,
		uint32_t height)
	{
		std::vector<uint8_t> buffer(width * height * 4);

		memcpy(buffer.data(), image1, buffer.size());
		memcpy(image1, image2, buffer.size());
		memcpy(image2, buffer.data(), buffer.size());
	}

	VkSampler CreateImageSampler(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		float mipLevels,
		VkSamplerAddressMode modeU,
		VkSamplerAddressMode modeV,
		VkSamplerAddressMode modeW)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = modeU;
		samplerInfo.addressModeV = modeV;
		samplerInfo.addressModeW = modeW;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy =
			properties.limits.maxSamplerAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = mipLevels;

		VkSampler sampler;

		VkResult res = vkCreateSampler(
			device,
			&samplerInfo,
			nullptr,
			&sampler);

		if (res != VK_SUCCESS) {
			throw std::runtime_error(
				"Failed to create image sampler.");
		}

		return sampler;
	}

	void DestroyImageSampler(VkDevice device, VkSampler sampler)
	{
		vkDestroySampler(device, sampler, nullptr);
	}
}
