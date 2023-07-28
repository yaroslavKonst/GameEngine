#include "ExternModel.h"

#include "../Utils/loader.h"
#include "../Logger/logger.h"

ExternModel::ExternModel(
	std::string modelFile,
	uint32_t textureIndex,
	uint32_t specularIndex,
	glm::mat4 matrix)
{
	auto model = Loader::LoadModel(modelFile);

	SetModelVertices(model.Vertices);
	SetModelNormals(model.Normals);
	SetModelTexCoords(model.TexCoords);
	SetModelIndices(model.Indices);

	SetObjectVertices(model.Vertices);
	SetObjectIndices(model.Indices);
	SetObjectCenter();

	SetTexture({textureIndex, specularIndex});

	SetModelMatrix(matrix);
	SetObjectMatrix(matrix);
	SetModelInnerMatrix(glm::mat4(1.0f));
	SetModelInstances({glm::mat4(1.0f)});

	SetDrawEnabled(true);
}
