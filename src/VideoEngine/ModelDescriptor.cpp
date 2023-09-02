#include "ModelDescriptor.h"

#include <string.h>

std::vector<VkVertexInputBindingDescription>
ModelDescriptor::GetVertexBindingDescription()
{
	std::vector<VkVertexInputBindingDescription> bindingDescription(2);
	memset(
		bindingDescription.data(),
		0,
		sizeof(VkVertexInputBindingDescription) *
		bindingDescription.size());

	bindingDescription[0].binding = 0;
	bindingDescription[0].stride = sizeof(Vertex);
	bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	bindingDescription[1].binding = 1;
	bindingDescription[1].stride = sizeof(glm::mat4);
	bindingDescription[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription>
ModelDescriptor::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription>
		attributeDescriptions(7);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, Pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, Normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, TexCoord);

	attributeDescriptions[3].binding = 1;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[3].offset = 0;

	attributeDescriptions[4].binding = 1;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = sizeof(glm::vec4);

	attributeDescriptions[5].binding = 1;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[5].offset = sizeof(glm::vec4) * 2;

	attributeDescriptions[6].binding = 1;
	attributeDescriptions[6].location = 6;
	attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[6].offset = sizeof(glm::vec4) * 3;

	return attributeDescriptions;
}

ModelDescriptor ModelDescriptor::CreateModelDescriptor(
	Loader::VertexData* model,
	VkDevice device,
	MemorySystem* memorySystem,
	PhysicalDeviceSupport* deviceSupport,
	VkQueue graphicsQueue,
	CommandPool* commandPool)
{
	ModelDescriptor descriptor;

	// Vertex buffer creation.
	auto& vertices = model->Vertices;
	auto& texCoords = model->TexCoords;
	auto& normals = model->Normals;

	std::vector<ModelDescriptor::Vertex> vertexData(vertices.size());

	descriptor.VertexBuffer = BufferHelper::CreateBuffer(
		device,
		sizeof(ModelDescriptor::Vertex) * vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		memorySystem,
		deviceSupport);

	for (size_t i = 0; i < vertices.size(); ++i) {
		vertexData[i].Pos = vertices[i];
		vertexData[i].TexCoord = texCoords[i];
		vertexData[i].Normal = normals[i];
	}

	BufferHelper::LoadDataToBuffer(
		device,
		descriptor.VertexBuffer,
		vertexData.data(),
		vertexData.size() * sizeof(ModelDescriptor::Vertex),
		memorySystem,
		deviceSupport,
		commandPool,
		graphicsQueue);

	descriptor.VertexCount = vertices.size();

	// Index buffer creation.
	auto& indices = model->Indices;

	descriptor.IndexBuffer = BufferHelper::CreateBuffer(
		device,
		sizeof(uint32_t) * indices.size(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		memorySystem,
		deviceSupport);

	BufferHelper::LoadDataToBuffer(
		device,
		descriptor.IndexBuffer,
		indices.data(),
		indices.size() * sizeof(uint32_t),
		memorySystem,
		deviceSupport,
		commandPool,
		graphicsQueue);

	descriptor.IndexCount = indices.size();

	// Instance buffer.
	auto& instances = model->Instances;

	descriptor.InstanceBuffer = BufferHelper::CreateBuffer(
		device,
		sizeof(glm::mat4) * instances.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		memorySystem,
		deviceSupport);

	BufferHelper::LoadDataToBuffer(
		device,
		descriptor.InstanceBuffer,
		instances.data(),
		instances.size() * sizeof(glm::mat4),
		memorySystem,
		deviceSupport,
		commandPool,
		graphicsQueue);

	descriptor.InstanceCount = instances.size();

	descriptor.MarkedFrameIndex = -1;

	return descriptor;
}

void ModelDescriptor::DestroyModelDescriptor(
	ModelDescriptor descriptor,
	VkDevice device,
	MemorySystem* memorySystem)
{
	BufferHelper::DestroyBuffer(
		device,
		descriptor.VertexBuffer,
		memorySystem);

	BufferHelper::DestroyBuffer(
		device,
		descriptor.IndexBuffer,
		memorySystem);

	BufferHelper::DestroyBuffer(
		device,
		descriptor.InstanceBuffer,
		memorySystem);
}
