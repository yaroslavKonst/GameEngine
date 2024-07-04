#ifndef _TEXTURE_HANDLER_H
#define _TEXTURE_HANDLER_H

#include <set>

#include "ImageHelper.h"
#include "PhysicalDeviceSupport.h"
#include "CommandPool.h"
#include "VkQueueObject.h"
#include "../Utils/RingBuffer.h"
#include "../Utils/ThreadPool.h"
#include "../Utils/loader.h"

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
		VkQueueObject* graphicsQueue,
		ThreadPool* threadPool);
	~TextureHandler();

	uint32_t AddTexture(
		const Loader::Image& image,
		bool repeat = true,
		bool async = false,
		TextureType type = TextureType::T2D,
		VkImageCreateFlagBits flags = (VkImageCreateFlagBits)0,
		uint32_t layerCount = 1);

	void RemoveTexture(uint32_t index);

	bool CheckTexture(uint32_t index)
	{
		return _textures.find(index) != _textures.end();
	}

	TextureDescriptor& GetTexture(uint32_t index)
	{
		return _textures[index];
	}

	void PollTextureMessages();

private:
	struct LoadTextureMessage
	{
		uint32_t Index;
		TextureDescriptor Descriptor;
	};

	struct RemoveTextureMessage
	{
		uint32_t Index;
	};

	RingBuffer<LoadTextureMessage> _loadMessages;
	RingBuffer<RemoveTextureMessage> _removeMessages;

	std::map<uint32_t, TextureDescriptor> _textures;
	std::set<uint32_t> _usedDescriptors;
	uint32_t _lastIndex;

	std::set<uint32_t> _descriptorsToDeleteOnReceive;

	VkDevice _device;
	PhysicalDeviceSupport* _deviceSupport;
	MemorySystem* _memorySystem;
	CommandPool* _commandPool;
	VkQueueObject* _graphicsQueue;

	ThreadPool* _threadPool;

	VkDescriptorSetLayout _descriptorSetLayout;

	Sync::Mutex _texAddMutex;

	TextureDescriptor CreateTextureDescriptor(
		TextureType type,
		const Loader::Image& image,
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
