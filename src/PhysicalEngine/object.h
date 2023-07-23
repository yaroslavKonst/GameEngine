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
	struct CollisionPrimitive
	{
		glm::vec3 Vertices[4];
	};

	Object()
	{
		_initialized = false;
		_effect = glm::vec3(0.0f);
	}

	virtual ~Object()
	{
	}

	virtual const std::vector<CollisionPrimitive>& GetCollisionPrimitives()
	{
		return _collisionPrimitives;
	}

	virtual void SetCollisionPrimitives(
		const std::vector<CollisionPrimitive>& value)
	{
		_collisionPrimitives = value;
	}

	virtual const glm::mat4& GetObjectMatrix()
	{
		return _matrix;
	}

	virtual void SetObjectMatrix(const glm::mat4& value)
	{
		_matrix = value;
	}

	virtual const glm::vec3& _GetObjectCenter()
	{
		return _center;
	}

	virtual void _SetObjectCenter(const glm::vec3& value)
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

private:
	std::vector<CollisionPrimitive> _collisionPrimitives;
	glm::mat4 _matrix;
	glm::vec3 _center;
	float _radius;
	bool _initialized;
	glm::vec3 _effect;
};

#endif
