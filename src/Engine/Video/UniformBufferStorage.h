#ifndef _UNIFORM_BUFFER_STORAGE_H
#define _UNIFORM_BUFFER_STORAGE_H

#include "BufferHelper.h"
#include "../Math/mat.h"
#include "model.h"

class UniformBufferStorage
{
public:
	UniformBufferStorage(
		VkDevice device,
		MemorySystem* memorySystem,
		PhysicalDeviceSupport* deviceSupport);
	~UniformBufferStorage();

	void Allocate(const Model* model, uint32_t size);
	void Free(const Model* model);

	void Write(
		const Model* model,
		void* data,
		uint32_t size);
	void WriteAll(
		const Model* model,
		void* data,
		uint32_t size);

	VkDescriptorSetLayout GetDescriptorSetLayout()
	{
		return _descriptorSetLayout;
	}

	VkDescriptorSet GetDescriptorSet(const Model* model)
	{
		if (_buffers.find(model) == _buffers.end()) {
			return _buffers[0].DescriptorSet[_currentSet];
		}

		VkDescriptorSet res =
			_buffers[model].DescriptorSet[_currentSet];

		return res;
	}

	void SwitchSet()
	{
		_currentSet = (_currentSet + 1) % 2;
	}

	void UpdateBuffer(const Model& model, const Model* modelPointer);

private:
	struct BufferData
	{
		BufferHelper::Buffer Buffer[2];
		VkDescriptorPool DescriptorPool;
		VkDescriptorSet DescriptorSet[2];
	};

	VkDevice _device;
	MemorySystem* _memorySystem;
	PhysicalDeviceSupport* _deviceSupport;

	VkDescriptorSetLayout _descriptorSetLayout;

	std::map<const Model*, BufferData> _buffers;

	int _currentSet;

	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();

	BufferData CreateBuffer(uint32_t size);
	void DestroyBuffer(BufferData buffer);
};

#endif
