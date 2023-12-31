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

	virtual const std::vector<glm::vec3>& GetModelVertices()
	{
		return _modelVertexBuffer;
	}

	virtual const std::vector<uint32_t>& GetModelIndices()
	{
		return _modelIndexBuffer;
	}

	virtual void SetModelVertices(const std::vector<glm::vec3>& vertices)
	{
		_modelVertexBuffer = vertices;
	}

	virtual void SetModelIndices(const std::vector<uint32_t>& indices)
	{
		_modelIndexBuffer = indices;
	}

	const std::vector<glm::vec2>& GetModelTexCoords()
	{
		return _modelTexCoordBuffer;
	}

	void SetModelTexCoords(const std::vector<glm::vec2>& coords)
	{
		_modelTexCoordBuffer = coords;
	}

	const std::vector<glm::mat4>& GetModelInstances()
	{
		return _modelInstances;
	}

	void SetModelInstances(const std::vector<glm::mat4>& instances)
	{
		_modelInstances = instances;
	}

	virtual const glm::mat4& GetModelInnerMatrix()
	{
		return _modelInnerMatrix;
	}

	virtual void SetModelInnerMatrix(const glm::mat4& matrix)
	{
		_modelInnerMatrix = matrix;
	}

	virtual const std::vector<glm::vec3>& GetModelNormals()
	{
		return _modelNormalBuffer;
	}

	virtual void SetModelNormals(const std::vector<glm::vec3>& normals)
	{
		_modelNormalBuffer = normals;
	}

private:
	std::vector<glm::vec3> _modelVertexBuffer;
	std::vector<glm::vec3> _modelNormalBuffer;
	std::vector<uint32_t> _modelIndexBuffer;
	std::vector<glm::vec2> _modelTexCoordBuffer;
	std::vector<glm::mat4> _modelInstances;

	glm::mat4 _modelMatrix;
	glm::mat4 _modelInnerMatrix;
};

#endif
