#include "TextureHandler.h"

#include <cmath>
#include <cstring>

#define RING_BUFFER_SIZE 1024 * 1024

TextureHandler::TextureHandler(
	VkDevice device,
	PhysicalDeviceSupport* deviceSupport,
	MemorySystem* memorySystem,
	VkDescriptorSetLayout descriptorSetLayout,
	CommandPool* commandPool,
	VkQueueObject* graphicsQueue,
	ThreadPool* threadPool) :
	_loadMessages(RING_BUFFER_SIZE),
	_removeMessages(RING_BUFFER_SIZE)
{
	_device = device;
	_memorySystem = memorySystem;
	_descriptorSetLayout = descriptorSetLayout;
	_deviceSupport = deviceSupport;
	_commandPool = commandPool;
	_graphicsQueue = graphicsQueue;
	_threadPool = threadPool;

	_lastIndex = 0;
}

TextureHandler::~TextureHandler()
{
	for (auto& texture : _textures) {
		DestroyTextureDescriptor(texture.second);
	}
}

void TextureHandler::PollTextureMessages()
{
	while (!_loadMessages.IsEmpty()) {
		auto msg = _loadMessages.Get();

		if (
			_descriptorsToDeleteOnReceive.find(msg.Index) !=
			_descriptorsToDeleteOnReceive.end())
		{
			DestroyTextureDescriptor(msg.Descriptor);
			_descriptorsToDeleteOnReceive.erase(msg.Index);
		} else {
			_textures[msg.Index] = msg.Descriptor;
		}
	}

	while (!_removeMessages.IsEmpty()) {
		auto msg = _removeMessages.Get();

		if (_textures.find(msg.Index) == _textures.end()) {
			_descriptorsToDeleteOnReceive.insert(msg.Index);
		} else {
			DestroyTextureDescriptor(_textures[msg.Index]);
			_textures.erase(msg.Index);
		}
	}
}

uint32_t TextureHandler::AddTexture(
	const Loader::Image& image,
	bool repeat,
	bool async,
	TextureType type,
	VkImageCreateFlagBits flags,
	uint32_t layerCount)
{
	_texAddMutex.Lock();
	uint32_t index = _lastIndex + 1;

	while (_usedDescriptors.find(index) != _usedDescriptors.end()) {
		++index;
	}

	_lastIndex = index;
	_usedDescriptors.insert(index);
	_texAddMutex.Unlock();

	Loader::Image* imageData = new Loader::Image();
	*imageData = image;

	uint32_t id = _threadPool->Enqueue(
		[this,
		index,
		type,
		imageData,
		repeat,
		flags,
		layerCount]() -> void
		{
			_loadMessages.Insert({
				index,
				CreateTextureDescriptor(
					type,
					*imageData,
					repeat,
					flags,
					layerCount)});
			delete imageData;
		},
		!async);

	if (!async) {
		_threadPool->Wait(id);
	}

	return index;
}

void TextureHandler::RemoveTexture(uint32_t index)
{
	_removeMessages.Insert({index});
	_texAddMutex.Lock();
	_usedDescriptors.erase(index);
	_texAddMutex.Unlock();
}

TextureHandler::TextureDescriptor TextureHandler::CreateTextureDescriptor(
	TextureType type,
	const Loader::Image& image,
	bool repeat,
	VkImageCreateFlagBits flags,
	uint32_t layerCount)
{
	TextureDescriptor descriptor;

	uint32_t mipLevels;
	descriptor.Image = CreateTextureImage(
		image.Width,
		image.Height,
		image.PixelData,
		mipLevels,
		flags,
		layerCount);

	VkImageViewType vType = VK_IMAGE_VIEW_TYPE_2D;

	switch (type) {
	case TextureType::T2D:
		vType = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case TextureType::TCube:
		vType = VK_IMAGE_VIEW_TYPE_CUBE;
		break;
	}

	descriptor.ImageView = ImageHelper::CreateImageView(
		_device,
		descriptor.Image.Image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT,
		mipLevels,
		vType,
		layerCount);

	VkSamplerAddressMode addressMode =
		repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT :
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	descriptor.Sampler = ImageHelper::CreateImageSampler(
		_device,
		_deviceSupport->GetPhysicalDevice(),
		mipLevels,
		addressMode,
		addressMode,
		addressMode);

	CreateDescriptorSets(&descriptor);

	return descriptor;
}

void TextureHandler::DestroyTextureDescriptor(
	TextureDescriptor& descriptor)
{
	DestroyDescriptorSets(&descriptor);

	ImageHelper::DestroyImageSampler(_device, descriptor.Sampler);

	ImageHelper::DestroyImageView(_device, descriptor.ImageView);

	ImageHelper::DestroyImage(_device, descriptor.Image, _memorySystem);
}

void TextureHandler::CreateDescriptorSets(TextureDescriptor* descriptor)
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	VkResult res = vkCreateDescriptorPool(
		_device,
		&poolInfo,
		nullptr,
		&descriptor->DescriptorPool);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool.");
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptor->DescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_descriptorSetLayout;

	res = vkAllocateDescriptorSets(
		_device,
		&allocInfo,
		&descriptor->DescriptorSet);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor set.");
	}

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = descriptor->ImageView;
	imageInfo.sampler = descriptor->Sampler;

	VkWriteDescriptorSet descriptorWrite{};

	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptor->DescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(
		_device,
		1,
		&descriptorWrite,
		0,
		nullptr);
}

void TextureHandler::DestroyDescriptorSets(TextureDescriptor* descriptor)
{
	vkDestroyDescriptorPool(_device, descriptor->DescriptorPool, nullptr);
}

ImageHelper::Image TextureHandler::CreateTextureImage(
	uint32_t width,
	uint32_t height,
	const std::vector<uint8_t>& texture,
	uint32_t& mipLevels,
	VkImageCreateFlagBits flags,
	uint32_t layerCount)
{
	uint32_t texWidth = width;
	uint32_t texHeight = height;

	uint32_t imageSize = texWidth * texHeight * 4;

	mipLevels = static_cast<uint32_t>(
		std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (layerCount > 1) {
		mipLevels = 1;
	}

	ImageHelper::Image textureImage = ImageHelper::CreateImage(
		_device,
		texWidth,
		texHeight,
		mipLevels,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_memorySystem,
		_deviceSupport,
		flags,
		layerCount);

	BufferHelper::Buffer stagingBuffer = BufferHelper::CreateBuffer(
		_device,
		imageSize * layerCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		_memorySystem,
		_deviceSupport,
		true);

	memcpy(
		stagingBuffer.Allocation.Mapping,
		texture.data(),
		texture.size());

	ImageHelper::ChangeImageLayout(
		textureImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		mipLevels,
		_commandPool,
		_graphicsQueue,
		layerCount);

	ImageHelper::CopyBufferToImage(
		stagingBuffer,
		textureImage,
		texWidth,
		texHeight,
		_commandPool,
		_graphicsQueue,
		layerCount);

	BufferHelper::DestroyBuffer(
		_device,
		stagingBuffer,
		_memorySystem);

	if (layerCount == 1) {
		GenerateMipmaps(
			textureImage,
			texWidth,
			texHeight,
			mipLevels);
	} else {
		ImageHelper::ChangeImageLayout(
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			mipLevels,
			_commandPool,
			_graphicsQueue,
			layerCount);
	}

	return textureImage;
}

void TextureHandler::GenerateMipmaps(
	ImageHelper::Image image,
	uint32_t width,
	uint32_t height,
	uint32_t mipLevels)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(
		_deviceSupport->GetPhysicalDevice(),
		image.Format,
		&formatProperties);

	if (!(formatProperties.optimalTilingFeatures &
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error(
			"Image format does not support linear blitting.");
	}

	VkCommandBuffer commandBuffer =
		_commandPool->StartOneTimeBuffer();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image.Image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = static_cast<int32_t>(width);
	int32_t mipHeight = static_cast<int32_t>(height);

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = {
			mipWidth > 1 ? mipWidth / 2 : 1,
			mipHeight > 1 ? mipHeight / 2 : 1,
			1
		};

		blit.dstSubresource.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			commandBuffer,
			image.Image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image.Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		if (mipWidth > 1) {
			mipWidth /= 2;
		}

		if (mipHeight > 1) {
			mipHeight /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier);

	_commandPool->EndOneTimeBuffer(commandBuffer, _graphicsQueue);
}
