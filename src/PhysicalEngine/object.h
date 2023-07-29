#ifndef _OBJECT_H
#define _OBJECT_H

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

class Object
{
public:
	Object()
	{
		_initialized = false;
		_effect = glm::vec3(0.0f);
		_speed = glm::vec3(0.0f);
		_dynamic = false;
	}

	virtual ~Object()
	{
	}

	virtual void SetObjectVertices(
		const std::vector<glm::vec3>& value)
	{
		_collisionVertices = value;
	}

	virtual const std::vector<glm::vec3>& GetObjectVertices()
	{
		return _collisionVertices;
	}

	virtual void SetObjectIndices(
		const std::vector<uint32_t>& value)
	{
		_collisionIndices = value;
	}

	virtual const std::vector<uint32_t>& GetObjectIndices()
	{
		return _collisionIndices;
	}

	virtual const glm::mat4& GetObjectMatrix()
	{
		return _matrix;
	}

	virtual void SetObjectMatrix(const glm::mat4& value)
	{
		_matrix = value;
	}

	virtual const glm::vec3& GetObjectCenter()
	{
		return _center;
	}

	virtual void SetObjectCenter(const glm::vec3& value)
	{
		_center = value;
	}

	virtual float _GetObjectRadius()
	{
		return _radius;
	}

	virtual void _SetObjectRadius(float value)
	{
		_radius = value;
	}

	virtual bool _IsObjectInitialized()
	{
		return _initialized;
	}

	virtual void _SetObjectInitialized(bool value)
	{
		_initialized = value;
	}

	virtual const glm::vec3& GetObjectEffect()
	{
		return _effect;
	}

	virtual void SetObjectEffect(const glm::vec3& value)
	{
		_effect = value;
	}

	virtual void IncObjectEffect(const glm::vec3& value)
	{
		_effect += value;
	}

	virtual const glm::vec3& GetObjectSpeed()
	{
		return _speed;
	}

	virtual void SetObjectSpeed(const glm::vec3& value)
	{
		_speed = value;
	}

	virtual void SetObjectCenter()
	{
		_center = glm::vec3(0.0f);

		for (auto& vertex : _collisionVertices) {
			_center += vertex;
		}

		_center /= _collisionVertices.size();
	}

	virtual bool IsObjectDynamic()
	{
		return _dynamic;
	}

	virtual void SetObjectDynamic(bool value)
	{
		_dynamic = value;
	}

	virtual void RayCastCallback(void* userPointer)
	{
	}

private:
	std::vector<glm::vec3> _collisionVertices;
	std::vector<uint32_t> _collisionIndices;
	glm::mat4 _matrix;
	glm::vec3 _speed;
	glm::vec3 _center;
	float _radius;
	bool _initialized;
	glm::vec3 _effect;
	bool _dynamic;
};

#endif
