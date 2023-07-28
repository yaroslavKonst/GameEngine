#ifndef _EXTERN_MODEL_H
#define _EXTERN_MODEL_H

#include <string>

#include "../VideoEngine/model.h"
#include "../PhysicalEngine/object.h"

class ExternModel : public Model, public Object
{
public:
	ExternModel(
		std::string modelFile,
		uint32_t textureIndex,
		uint32_t specularIndex,
		glm::mat4 matrix);
};

#endif
