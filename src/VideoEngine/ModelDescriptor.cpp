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
		attributeDescriptions(6);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, Pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, TexCoord);

	attributeDescriptions[2].binding = 1;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[2].offset = 0;

	attributeDescriptions[3].binding = 1;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[3].offset = sizeof(glm::vec4);

	attributeDescriptions[4].binding = 1;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = sizeof(glm::vec4) * 2;

	attributeDescriptions[5].binding = 1;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[5].offset = sizeof(glm::vec4) * 3;

	return attributeDescriptions;
}
