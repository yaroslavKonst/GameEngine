#ifndef _TEXTURE_HANDLER_H
#define _TEXTURE_HANDLER_H

#include "ImageHelper.h"
#include "PhysicalDeviceSupport.h"
#include "CommandPool.h"
#include "VkQueueObject.h"

class TextureHandler
{
public:
	enum class TextureType
	{
		T2D,
		TCube
	};

	struct TextureDescriptor
	{
		ImageHelper::Image Image;
		VkImageView ImageView;
		VkSampler Sampler;

		VkDescriptorPool DescriptorPool;
		VkDescriptorSet DescriptorSet;
	};

	TextureHandler(
		VkDevice device,
		PhysicalDeviceSupport* deviceSupport,
		MemorySystem* memorySystem,
		VkDescriptorSetLayout descriptorSetLayout,
		CommandPool* commandPool,
		VkQueueObject* graphicsQueue);
	~TextureHandler();

	uint32_t AddTexture(
		uint32_t width,
		uint32_t height,
		const std::vector<uint8_t>& texture,
		bool repeat = true,
		TextureType type = TextureType::T2D,
		VkImageCreateFlagBits flags = (VkImageCreateFlagBits)0,
		uint32_t layerCount = 1);

	void RemoveTexture(uint32_t index);

	TextureDescriptor& GetTexture(uint32_t index)
	{
		return _textures[index];
	}

private:
	std::map<uint32_t, TextureDescriptor> _textures;
	uint32_t _lastIndex;

	VkDevice _device;
	PhysicalDeviceSupport* _deviceSupport;
	MemorySystem* _memorySystem;
	CommandPool* _commandPool;
	VkQueueObject* _graphicsQueue;

	VkDescriptorSetLayout _descriptorSetLayout;

	TextureDescriptor CreateTextureDescriptor(
		TextureType type,
		uint32_t width,
		uint32_t height,
		const std::vector<uint8_t>& texture,
		bool repeat,
		VkImageCreateFlagBits flags = (VkImageCreateFlagBits)0,
		uint32_t layerCount = 1);
	void DestroyTextureDescriptor(TextureDescriptor& descriptor);

	void CreateDescriptorSets(TextureDescriptor* descriptor);
	void DestroyDescriptorSets(TextureDescriptor* descriptor);

	ImageHelper::Image CreateTextureImage(
		uint32_t width,
		uint32_t height,
		const std::vector<uint8_t>& texture,
		uint32_t& mipLevels,
		VkImageCreateFlagBits flags = (VkImageCreateFlagBits)0,
		uint32_t layerCount = 1);

	void GenerateMipmaps(
		ImageHelper::Image image,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels);
};

#endif
