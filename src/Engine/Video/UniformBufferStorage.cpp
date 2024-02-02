#include "UniformBufferStorage.h"

#include <cstring>
#include <array>

#include "glm.h"

static glm::mat4 MatToGlm(const Math::Mat<4>& mat)
{
	glm::mat4 res;

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			res[row][col] = mat[col][row];
		}
	}

	return res;
}

UniformBufferStorage::UniformBufferStorage(
	VkDevice device,
	MemorySystem* memorySystem,
	PhysicalDeviceSupport* deviceSupport)
{
	_device = device;
	_memorySystem = memorySystem;
	_deviceSupport = deviceSupport;

	_currentSet = 0;

	CreateDescriptorSetLayout();

	_buffers[0] = CreateBuffer(sizeof(glm::mat4));
	glm::mat4 matrix(1.0);
	WriteAll(0, &matrix, sizeof(matrix));
}

UniformBufferStorage::~UniformBufferStorage()
{
	for (auto& buffer : _buffers) {
		DestroyBuffer(buffer.second);
	}

	DestroyDescriptorSetLayout();
}

void UniformBufferStorage::Allocate(const Model* model, uint32_t size)
{
	_buffers[model] = CreateBuffer(size);
}

void UniformBufferStorage::Free(const Model* model)
{
	if (_buffers.find(model) == _buffers.end()) {
		return;
	}

	DestroyBuffer(_buffers[model]);
	_buffers.erase(model);
}

void UniformBufferStorage::Write(
	const Model* model,
	void* data,
	uint32_t size)
{
	BufferData buffer = _buffers[model];

	memcpy(
		buffer.Buffer[_currentSet].Allocation.Mapping,
		data,
		size);
}

void UniformBufferStorage::WriteAll(
	const Model* model,
	void* data,
	uint32_t size)
{
	BufferData buffer = _buffers[model];

	for (int i = 0; i < 2; ++i) {
		memcpy(
			buffer.Buffer[i].Allocation.Mapping,
			data,
			size);
	}
}

void UniformBufferStorage::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding bufferLayoutBinding{};
	bufferLayoutBinding.binding = 0;
	bufferLayoutBinding.descriptorCount = 1;
	bufferLayoutBinding.descriptorType =
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bufferLayoutBinding.pImmutableSamplers = nullptr;
	bufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
		bufferLayoutBinding,
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult res = vkCreateDescriptorSetLayout(
		_device,
		&layoutInfo,
		nullptr,
		&_descriptorSetLayout);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to create buffer descriptor set layout.");
	}
}

void UniformBufferStorage::DestroyDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(
		_device,
		_descriptorSetLayout,
		nullptr);
}

UniformBufferStorage::BufferData UniformBufferStorage::CreateBuffer(
	uint32_t size)
{
	BufferData buffer;

	for (int i = 0; i < 2; ++i) {
		buffer.Buffer[i] = BufferHelper::CreateBuffer(
			_device,
			size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			_memorySystem,
			_deviceSupport,
			true);
	}

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 2;

	VkResult res = vkCreateDescriptorPool(
		_device,
		&poolInfo,
		nullptr,
		&buffer.DescriptorPool);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to create buffer descriptor pool.");
	}

	std::array<VkDescriptorSetLayout, 2> setLayouts = {
		_descriptorSetLayout,
		_descriptorSetLayout
	};

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = buffer.DescriptorPool;
	allocInfo.descriptorSetCount = 2;
	allocInfo.pSetLayouts = setLayouts.data();

	res = vkAllocateDescriptorSets(
		_device,
		&allocInfo,
		buffer.DescriptorSet);

	if (res != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to create buffer descriptor set.");
	}

	VkDescriptorBufferInfo bufferInfo[2];
	memset(bufferInfo, 0, sizeof(bufferInfo));

	for (int i = 0; i < 2; ++i) {
		bufferInfo[i].buffer = buffer.Buffer[i].Buffer;
		bufferInfo[i].offset = 0;
		bufferInfo[i].range = size;
	}

	VkWriteDescriptorSet descriptorWrite[2];
	memset(descriptorWrite, 0, sizeof(descriptorWrite));

	for (int i = 0; i < 2; ++i) {
		descriptorWrite[i].sType =
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[i].dstSet = buffer.DescriptorSet[i];
		descriptorWrite[i].dstBinding = 0;
		descriptorWrite[i].dstArrayElement = 0;
		descriptorWrite[i].descriptorType =
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[i].descriptorCount = 1;
		descriptorWrite[i].pBufferInfo = bufferInfo + i;
	}

	vkUpdateDescriptorSets(
		_device,
		2,
		descriptorWrite,
		0,
		nullptr);

	return buffer;
}

void UniformBufferStorage::DestroyBuffer(BufferData buffer)
{
	vkDestroyDescriptorPool(
		_device,
		buffer.DescriptorPool,
		nullptr);

	for (int i = 0; i < 2; ++i) {
		BufferHelper::DestroyBuffer(
			_device,
			buffer.Buffer[i],
			_memorySystem);
	}
}

void UniformBufferStorage::UpdateBuffer(
	const Model& model,
	const Model* modelPointer)
{
	size_t matrixCount = model.ModelParams.InnerMatrix.size();

	if (!matrixCount) {
		return;
	}

	std::vector<glm::mat4> matrices(matrixCount);

	for (size_t matIdx = 0; matIdx < matrixCount; ++matIdx) {
		matrices[matIdx] =
			MatToGlm(model.ModelParams.InnerMatrix[matIdx]);
	}

	if (_buffers.find(modelPointer) == _buffers.end()) {
		Allocate(
			modelPointer,
			sizeof(glm::mat4) * matrixCount);
	}

	Write(
		modelPointer,
		matrices.data(),
		sizeof(glm::mat4) * matrixCount);
}
