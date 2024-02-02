#include "loader.h"

#include <cstring>
#include <map>
#include <array>

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"

#include "TextFileParser.h"
#include "../Logger/logger.h"
#include "../Assets/package.h"

namespace Loader
{
	Image LoadImage(std::string name)
	{
		int channels;
		int width;
		int height;

		auto imageData = Package::Instance()->GetData(name);

		stbi_uc* pixels = stbi_load_from_memory(
			imageData.data(),
			imageData.size(),
			&width,
			&height,
			&channels,
			STBI_rgb_alpha);

		if (!pixels) {
			throw std::runtime_error(
				"Failed to load image.");
		}

		Image image;
		image.Width = width;
		image.Height = height;
		image.PixelData.resize(width * height * 4);
		memcpy(image.PixelData.data(), pixels, image.PixelData.size());

		stbi_image_free(pixels);

		return image;
	}

	VertexData LoadModel(std::string name)
	{
		struct Vertex
		{
			Math::Vec<3> Pos;
			Math::Vec<2> Tex;
			Math::Vec<3> Normal;
			VertexData::MatrixIndex MatrixIndex;

			bool operator<(const Vertex& vertex) const
			{
				for (int i = 0; i < 3; ++i) {
					if (Pos[i] != vertex.Pos[i]) {
						return Pos[i] < vertex.Pos[i];
					}
				}

				for (int i = 0; i < 2; ++i) {
					if (Tex[i] != vertex.Tex[i]) {
						return Tex[i] < vertex.Tex[i];
					}
				}

				for (int i = 0; i < 3; ++i) {
					if (Normal[i] != vertex.Normal[i]) {
						return Normal[i] <
							vertex.Normal[i];
					}
				}

				for (int i = 0; i < 2; ++i) {
					if (MatrixIndex.Index[i] !=
						vertex.MatrixIndex.Index[i])
					{
						return MatrixIndex.Index[i] <
							vertex.MatrixIndex.Index[i];
					}
				}

				for (int i = 0; i < 2; ++i) {
					if (MatrixIndex.Coeff[i] !=
						vertex.MatrixIndex.Coeff[i])
					{
						return MatrixIndex.Coeff[i] <
							vertex.MatrixIndex.Coeff[i];
					}
				}

				return false;
			}
		};

		VertexData data;

		auto modelFile = TextFileParser::ParseFile(
			name,
			{' ', '/'});

		std::vector<Vertex> vertices;

		std::vector<Math::Vec<3>> vertexPositions;
		std::vector<Math::Vec<3>> vertexNormals;
		std::vector<Math::Vec<2>> vertexTexCoords;
		std::vector<std::array<uint32_t, 2>> vertexGroups;
		std::vector<std::array<float, 2>> vertexCoeffs;

		std::map<std::string, uint32_t> vertexGroupId;
		uint32_t nextVertexGroupId = 0;
		std::string currentGroupName = "";

		uint32_t interpolateId[2] = {0, 0};
		float interpolateCoeffs[2] = {1.0, 0.0};

		for (auto line : modelFile) {
			if (line[0] == "v") {
				Math::Vec<3> vertex;
				vertex[0] = std::stod(line[1]);
				vertex[1] = std::stod(line[2]);
				vertex[2] = std::stod(line[3]);
				vertexPositions.push_back(vertex);

				std::array<uint32_t, 2> vg;
				std::array<float, 2> vc;

				if (currentGroupName == "%Interpolate") {
					for (int i = 0; i < 2; ++i) {
						vg[i] = interpolateId[i];
						vc[i] = interpolateCoeffs[i];
					}
				} else {
					bool groupDefined =
						vertexGroupId.find(
							currentGroupName) !=
						vertexGroupId.end();

					if (!groupDefined) {
						vertexGroupId[currentGroupName] =
							nextVertexGroupId;
						++nextVertexGroupId;
					}

					vg[0] = vertexGroupId[currentGroupName];
					vg[1] = vertexGroupId[currentGroupName];

					vc[0] = 1;
					vc[1] = 0;
				}

				vertexGroups.push_back(vg);
				vertexCoeffs.push_back(vc);
			} else if (line[0] == "vt") {
				Math::Vec<2> texCoord;
				texCoord[0] = std::stod(line[1]);
				texCoord[1] = 1.0 - std::stod(line[2]);
				vertexTexCoords.push_back(texCoord);
			} else if (line[0] == "vn") {
				Math::Vec<3> normal;
				normal[0] = std::stod(line[1]);
				normal[1] = std::stod(line[2]);
				normal[2] = std::stod(line[3]);
				vertexNormals.push_back(normal);
			} else if (line[0] == "f") {
				for (int i = 1; i <= 9; i += 3) {
					Vertex vertex;

					vertex.MatrixIndex.Index = vertexGroups[
						std::stoi(line[i]) - 1];
					vertex.MatrixIndex.Coeff = vertexCoeffs[
						std::stoi(line[i]) - 1];
					vertex.Pos = vertexPositions[
						std::stoi(line[i]) - 1];
					vertex.Tex = vertexTexCoords[
						std::stoi(line[i + 1]) - 1];
					vertex.Normal = vertexNormals[
						std::stoi(line[i + 2]) - 1];
					vertices.push_back(vertex);
				}
			} else if (line[0] == "vg") {
				currentGroupName = line[1];

				if (line[1] == "%Interpolate") {
					std::string gName1 = line[2];
					std::string gName2 = line[3];

					bool groupDefined =
						vertexGroupId.find(gName1) !=
						vertexGroupId.end();

					if (!groupDefined) {
						vertexGroupId[gName1] =
							nextVertexGroupId;
						++nextVertexGroupId;
					}

					groupDefined =
						vertexGroupId.find(gName2) !=
						vertexGroupId.end();

					if (!groupDefined) {
						vertexGroupId[gName2] =
							nextVertexGroupId;
						++nextVertexGroupId;
					}

					interpolateId[0] =
						vertexGroupId[gName1];
					interpolateId[1] =
						vertexGroupId[gName2];

					interpolateCoeffs[1] =
						std::stof(line[4]);

					interpolateCoeffs[0] =
						1.0f - interpolateCoeffs[1];
				}
			}
		}

		std::map<Vertex, uint32_t> indexedVertices;
		uint32_t maxIndex = 0;

		for (auto vertex : vertices) {
			auto vert = indexedVertices.find(vertex);

			if (vert == indexedVertices.end()) {
				indexedVertices[vertex] = maxIndex;
				++maxIndex;

				data.Vertices.push_back(vertex.Pos);
				data.TexCoords.push_back(vertex.Tex);
				data.Normals.push_back(vertex.Normal);
				data.MatrixIndices.push_back(
					vertex.MatrixIndex);
			}

			data.Indices.push_back(indexedVertices[vertex]);
		}

		data.Instances = {Math::Mat<4>(1.0)};

		return data;
	}
};
