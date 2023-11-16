#include "model.h"

Model::Model()
{
	_holed = false;
	_externMatrix = nullptr;
	_modelInnerMatrix = glm::mat4(1.0);
}

Model::~Model()
{
}
