#ifndef _LOADER_H
#define _LOADER_H

#include <string>
#include <stdexcept>
#include <vector>
#include <cstdint>

#include "../Math/vec.h"
#include "../Math/mat.h"

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
		std::vector<Math::Vec<3>> Vertices;
		std::vector<Math::Vec<3>> Normals;
		std::vector<Math::Vec<2>> TexCoords;
		std::vector<uint32_t> MatrixIndices;
		std::vector<uint32_t> Indices;

		std::vector<Math::Mat<4>> Instances;
	};

	VertexData LoadModel(std::string name);
}

#endif
