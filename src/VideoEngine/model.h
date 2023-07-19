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

class Model
{
public:
	Model();
	virtual ~Model();

	virtual const glm::mat4& GetModelMatrix()
	{
		return _modelMatrix;
	}

	virtual const std::vector<glm::vec2>& GetModelVertices()
	{
		return _modelVertexBuffer;
	}

	virtual const std::vector<glm::vec3>& GetModelColors()
	{
		return _modelColorBuffer;
	}

	virtual void SetModelVertices(const std::vector<glm::vec2>& vertices)
	{
		_modelVertexBuffer = vertices;
	}

	virtual void SetModelColors(const std::vector<glm::vec3>& colors)
	{
		_modelColorBuffer = colors;
	}
	virtual bool IsModelActive()
	{
		return _modelActive;
	}

	virtual void SetModelActive(bool active)
	{
		_modelActive = active;
	}

private:
	std::vector<glm::vec2> _modelVertexBuffer;
	std::vector<glm::vec3> _modelColorBuffer;
	std::vector<uint32_t> _modelIndexBuffer;
	std::vector<glm::vec2> _modelTexCoordBuffer;

	glm::mat4 _modelMatrix;

	bool _modelActive;
};

#endif
