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

	virtual bool IsModelActive()
	{
		return _modelActive && _ready;
	}

	virtual void SetModelActive(bool active)
	{
		_modelActive = active;
	}

	virtual void SetModelReady(bool ready)
	{
		_ready = ready;
	}

	uint32_t GetTexWidth()
	{
		return _texWidth;
	}

	uint32_t GetTexHeight()
	{
		return _texHeight;
	}

	const std::vector<uint8_t>& GetTexData()
	{
		return _modelTexture;
	}

	void SetTexWidth(uint32_t value)
	{
		_texWidth = value;
	}

	void SetTexHeight(uint32_t value)
	{
		_texHeight = value;
	}

	void SetTexData(const std::vector<uint8_t>& texture)
	{
		_modelTexture = texture;
	}

	const std::vector<glm::vec2>& GetTexCoords()
	{
		return _modelTexCoordBuffer;
	}

	void SetTexCoords(const std::vector<glm::vec2>& coords)
	{
		_modelTexCoordBuffer = coords;
	}

private:
	std::vector<glm::vec3> _modelVertexBuffer;
	std::vector<uint32_t> _modelIndexBuffer;
	std::vector<glm::vec2> _modelTexCoordBuffer;

	uint32_t _texWidth;
	uint32_t _texHeight;
	std::vector<uint8_t> _modelTexture;

	glm::mat4 _modelMatrix;

	bool _modelActive;
	bool _ready;
};

#endif
