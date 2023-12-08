#ifndef _PHYSICAL_OBJECT_H
#define _PHYSICAL_OBJECT_H

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

class PhysicalObject
{
public:
	struct PhysicalValues
	{
		bool Enabled;

		std::vector<glm::vec3> Vertices;
		std::vector<glm::vec3> Normals;
		std::vector<uint32_t> Indices;
		glm::mat4 Matrix;
		glm::mat4* ExternMatrix;

		bool Dynamic;
		uint32_t Domain;

		float Mass;
		glm::vec3 Speed;
		glm::vec3 ImpulseMoment;
		float Mu;
		bool ModifyExternalMatrix;
	};

	PhysicalValues PhysicsParams;

	PhysicalObject()
	{
		PhysicsParams.Enabled = false;
		_initialized = false;
		PhysicsParams.Dynamic = false;
		PhysicsParams.Domain = 0;
		PhysicsParams.ExternMatrix = nullptr;
		PhysicsParams.ModifyExternalMatrix = false;
	}

	virtual ~PhysicalObject()
	{ }

	float _GetObjectDiameter()
	{
		return _diameter;
	}

	void _SetObjectDiameter(float value)
	{
		_diameter = value;
	}

	virtual uint32_t RayCastCallback(void* userPointer)
	{
		return 0;
	}

private:
	float _diameter;
};

#endif
