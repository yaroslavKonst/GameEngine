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
	Model();
	virtual ~Model();

	virtual const glm::mat4& GetModelMatrix()
	{
		return _modelMatrix;
	}

	virtual void SetModelMatrix(const glm::mat4& matrix)
	{
		_modelMatrix = matrix;
	}

	virtual const std::vector<uint32_t>& GetModels()
	{
		return _models;
	}

	virtual void SetModels(const std::vector<uint32_t>& models)
	{
		_models = models;
	}

	virtual const glm::mat4& GetModelInnerMatrix()
	{
		return _modelInnerMatrix;
	}

	virtual void SetModelInnerMatrix(const glm::mat4& matrix)
	{
		_modelInnerMatrix = matrix;
	}

	virtual const glm::vec3& GetModelCenter()
	{
		return _modelCenter;
	}

	virtual void SetModelCenter(const glm::vec3& value)
	{
		_modelCenter = value;
	}

	virtual bool IsModelHoled()
	{
		return _holed;
	}

	virtual void SetModelHoled(bool value)
	{
		_holed = value;
	}

	virtual const glm::mat4* GetModelExternalMatrix()
	{
		return _externMatrix;
	}

	virtual void SetModelExternalMatrix(const glm::mat4* matrix)
	{
		_externMatrix = matrix;
	}

private:
	std::vector<uint32_t> _models;
	bool _holed;

	glm::mat4 _modelMatrix;
	glm::mat4 _modelInnerMatrix;
	const glm::mat4* _externMatrix;

	glm::vec3 _modelCenter;
};

#endif
