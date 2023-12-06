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
	struct Image
	{
		uint32_t Width;
		uint32_t Height;
		std::vector<uint8_t> PixelData;
	};

	Image LoadImage(std::string name);

	struct VertexData
	{
		std::vector<glm::vec3> Vertices;
		std::vector<glm::vec3> Normals;
		std::vector<glm::vec2> TexCoords;
		std::vector<uint32_t> Indices;

		std::vector<glm::mat4> Instances;
	};

	VertexData LoadModel(std::string name);
}

#endif
