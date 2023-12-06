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
			glm::vec3 Pos;
			glm::vec2 Tex;
			glm::vec3 Normal;

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

				return false;
			}
		};

		VertexData data;

		auto modelFile = TextFileParser::ParseFile(
			name,
			{' ', '/'});

		std::vector<Vertex> vertices;

		std::vector<glm::vec3> vertPositions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;

		for (auto line : modelFile) {
			if (line[0] == "v") {
				glm::vec3 vertex;
				vertex[0] = std::stof(line[1]);
				vertex[1] = std::stof(line[2]);
				vertex[2] = std::stof(line[3]);
				vertPositions.push_back(vertex);
			} else if (line[0] == "vt") {
				glm::vec2 texCoord;
				texCoord[0] = std::stof(line[1]);
				texCoord[1] = 1.0 - std::stof(line[2]);
				texCoords.push_back(texCoord);
			} else if (line[0] == "vn") {
				glm::vec3 normal;
				normal[0] = std::stof(line[1]);
				normal[1] = std::stof(line[2]);
				normal[2] = std::stof(line[3]);
				normals.push_back(normal);
			} else if (line[0] == "f") {
				Vertex vertex;
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
			}

			data.Indices.push_back(indexedVertices[vertex]);
		}

		data.Instances = {glm::mat4(1.0)};

		return data;
	}
};
