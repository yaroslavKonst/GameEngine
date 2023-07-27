#include "ExternModel.h"

#include "../Utils/loader.h"
#include "../Logger/logger.h"

ExternModel::ExternModel(
	std::string modelFile,
	std::string textureFile,
	glm::mat4 matrix)
{
	auto model = Loader::LoadModel(modelFile);

	int texWidth;
	int texHeight;
	auto texture = Loader::LoadImage(textureFile, texWidth, texHeight);

	SetModelVertices(model.Vertices);
	SetModelNormals(model.Normals);
	SetModelTexCoords(model.TexCoords);
	SetModelIndices(model.Indices);

	SetObjectVertices(model.Vertices);
	SetObjectIndices(model.Indices);
	SetObjectCenter();

	SetTexWidth(texWidth);
	SetTexHeight(texHeight);
	SetTexData(texture);

	SetModelMatrix(matrix);
	SetObjectMatrix(matrix);
	SetModelInnerMatrix(glm::mat4(1.0f));
	SetModelInstances({glm::mat4(1.0f)});

	SetDrawEnabled(true);
}
