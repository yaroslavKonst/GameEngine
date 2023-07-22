#include "ExternModel.h"

#include "../Utils/loader.h"
#include "../Logger/logger.h"

ExternModel::ExternModel(std::string modelFile, std::string textureFile)
{
	auto model = Loader::LoadModel(modelFile);

	int texWidth;
	int texHeight;
	auto texture = Loader::LoadImage(textureFile, texWidth, texHeight);

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
