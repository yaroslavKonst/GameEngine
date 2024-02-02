#ifndef _LOADER_H
#define _LOADER_H

#include <string>
#include <stdexcept>
#include <vector>
#include <array>
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
		struct MatrixIndex
		{
			std::array<uint32_t, 2> Index;
			std::array<float, 2> Coeff;
		};

		std::vector<Math::Vec<3>> Vertices;
		std::vector<Math::Vec<3>> Normals;
		std::vector<Math::Vec<2>> TexCoords;
		std::vector<MatrixIndex> MatrixIndices;
		std::vector<uint32_t> Indices;

		std::vector<Math::Mat<4>> Instances;
	};

	VertexData LoadModel(std::string name);
}

#endif
