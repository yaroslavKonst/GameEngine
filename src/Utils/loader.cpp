#include "loader.h"

#include <cstring>
#include <map>

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
			uint32_t MatrixIndex;

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

				return MatrixIndex < vertex.MatrixIndex;
			}
		};

		VertexData data;

		auto modelFile = TextFileParser::ParseFile(
			name,
			{' ', '/'});

		std::vector<Vertex> vertices;

		std::vector<Math::Vec<3>> vertPositions;
		std::vector<Math::Vec<3>> normals;
		std::vector<Math::Vec<2>> texCoords;

		for (auto line : modelFile) {
			if (line[0] == "v") {
				Math::Vec<3> vertex;
				vertex[0] = std::stod(line[1]);
				vertex[1] = std::stod(line[2]);
				vertex[2] = std::stod(line[3]);
				vertPositions.push_back(vertex);
			} else if (line[0] == "vt") {
				Math::Vec<2> texCoord;
				texCoord[0] = std::stod(line[1]);
				texCoord[1] = 1.0 - std::stod(line[2]);
				texCoords.push_back(texCoord);
			} else if (line[0] == "vn") {
				Math::Vec<3> normal;
				normal[0] = std::stod(line[1]);
				normal[1] = std::stod(line[2]);
				normal[2] = std::stod(line[3]);
				normals.push_back(normal);
			} else if (line[0] == "f") {
				Vertex vertex;
				vertex.MatrixIndex = 0;

				vertex.Pos = vertPositions[
					std::stoi(line[1]) - 1];
				vertex.Tex = texCoords[std::stoi(line[2]) - 1];
				vertex.Normal = normals[std::stoi(line[3]) - 1];
				vertices.push_back(vertex);

				vertex.Pos = vertPositions[
					std::stoi(line[4]) - 1];
				vertex.Tex = texCoords[std::stoi(line[5]) - 1];
				vertex.Normal = normals[std::stoi(line[6]) - 1];
				vertices.push_back(vertex);

				vertex.Pos = vertPositions[
					std::stoi(line[7]) - 1];
				vertex.Tex = texCoords[std::stoi(line[8]) - 1];
				vertex.Normal = normals[std::stoi(line[9]) - 1];
				vertices.push_back(vertex);
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
