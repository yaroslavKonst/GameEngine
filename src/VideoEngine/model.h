#ifndef _MODEL_H
#define _MODEL_H

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "texturable.h"

class Model : public Texturable
{
public:
	struct VideoModelValues
	{
		uint32_t Model;
		bool Holed;

		glm::mat4 Matrix;
		glm::mat4 InnerMatrix;
		const glm::mat4* ExternalMatrix;

		glm::vec3 Center;
	};

	VideoModelValues ModelParams;

	Model()
	{
		ModelParams.Holed = false;
		ModelParams.ExternalMatrix = nullptr;
		ModelParams.InnerMatrix = glm::mat4(1.0);
	}
};

#endif
