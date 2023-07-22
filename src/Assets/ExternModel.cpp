#include "ExternModel.h"

#include "../Utils/loader.h"
#include "../Logger/logger.h"

ExternModel::ExternModel(std::string modelFile, std::string textureFile)
{
	auto model = Loader::LoadModel(modelFile);

	int texWidth;
	int texHeight;
	auto texture = Loader::LoadImage(textureFile, texWidth, texHeight);

	Logger::Verbose() << "Loading model " << modelFile;

	for (uint32_t i = 0; i < model.Vertices.size(); ++i) {
		Logger::Verbose() << "Vertex " << i << ": " <<
			model.Vertices[i][0] << " " <<
			model.Vertices[i][1] << " " <<
			model.Vertices[i][2];
	}

	for (uint32_t i = 0; i < model.Normals.size(); ++i) {
		Logger::Verbose() << "Normal " << i << ": " <<
			model.Normals[i][0] << " " <<
			model.Normals[i][1] << " " <<
			model.Normals[i][2];
	}

	for (uint32_t i = 0; i < model.TexCoords.size(); ++i) {
		Logger::Verbose() << "TexCoord " << i << ": " <<
			model.TexCoords[i][0] << " " <<
			model.TexCoords[i][1];
	}

	for (uint32_t i = 0; i < model.Indices.size(); ++i) {
		Logger::Verbose() << "Index " << i << ": " <<
			model.Indices[i];
	}

	SetModelVertices(model.Vertices);
	SetModelNormals(model.Normals);
	SetModelTexCoords(model.TexCoords);
	SetModelIndices(model.Indices);

	SetTexWidth(texWidth);
	SetTexHeight(texHeight);
	SetTexData(texture);

	SetModelMatrix(glm::mat4(1.0f));
	SetModelInnerMatrix(glm::mat4(1.0f));
	SetModelInstances({glm::mat4(1.0f)});
}
