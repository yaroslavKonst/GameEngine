#ifndef _LOADER_H
#define _LOADER_H

#include <string>
#include <stdexcept>
#include <vector>
#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace Loader
{
	std::vector<uint8_t> LoadImage(
		std::string name,
		int& width,
		int& height);

	struct VertexData
	{
		std::vector<glm::vec3> Vertices;
		std::vector<glm::vec3> Normals;
		std::vector<glm::vec2> TexCoords;
		std::vector<uint32_t> Indices;
	};

	VertexData LoadModel(std::string name);
}

#endif
