#ifndef _EXTERN_MODEL_H
#define _EXTERN_MODEL_H

#include <string>

#include "../VideoEngine/model.h"

class ExternModel : public Model
{
public:
	ExternModel(std::string modelFile, std::string textureFile);
};

#endif
