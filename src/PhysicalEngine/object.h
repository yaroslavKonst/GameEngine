#ifndef _OBJECT_H
#define _OBJECT_H

#include <vector>

#include "../VideoEngine/glm.h"

class Object
{
public:
	Object()
	{
		_initialized = false;
		_effect = glm::vec3(0.0f);
		_effectSource = nullptr;
		_dynamic = false;
		_dynamicSphereRadius = 1;
		_dynamicSphereCenter = {0, 0, 0};
		_domain = 0;
		_externMatrix = nullptr;
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

	virtual void SetObjectNormals(
		const std::vector<glm::vec3>& value)
	{
		_collisionNormals = value;
	}

	virtual const std::vector<glm::vec3>& GetObjectNormals()
	{
		return _collisionNormals;
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

	virtual Object* GetObjectEffectSource()
	{
		return _effectSource;
	}

	virtual void SetObjectEffect(const glm::vec3& value)
	{
		_effect = value;
		_effectSource = nullptr;
	}

	virtual void IncObjectEffect(const glm::vec3& value, Object* object)
	{
		if (glm::length(value) > glm::length(_effect)) {
			_effect = value;
			_effectSource = object;
		}
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

	virtual void SetObjectSphereRadius(float value)
	{
		_dynamicSphereRadius = value;
	}

	virtual float GetObjectSphereRadius()
	{
		return _dynamicSphereRadius;
	}

	virtual void SetObjectSphereCenter(const glm::vec3& value)
	{
		_dynamicSphereCenter = value;
	}

	virtual const glm::vec3& GetObjectSphereCenter()
	{
		return _dynamicSphereCenter;
	}

	virtual uint32_t GetObjectDomain()
	{
		return _domain;
	}

	virtual void SetObjectDomain(uint32_t value)
	{
		_domain = value;
	}

	virtual glm::mat4* GetObjectExternalMatrix()
	{
		return _externMatrix;
	}

	virtual void SetObjectExternalMatrix(glm::mat4* value)
	{
		_externMatrix = value;
	}

	virtual uint32_t RayCastCallback(void* userPointer)
	{
		return 0;
	}

private:
	std::vector<glm::vec3> _collisionVertices;
	std::vector<glm::vec3> _collisionNormals;
	std::vector<uint32_t> _collisionIndices;
	glm::mat4 _matrix;
	glm::vec3 _center;
	float _radius;
	bool _initialized;
	glm::vec3 _effect;
	Object* _effectSource;

	bool _dynamic;
	float _dynamicSphereRadius;
	glm::vec3 _dynamicSphereCenter;

	uint32_t _domain;

	glm::mat4* _externMatrix;
};

#endif
