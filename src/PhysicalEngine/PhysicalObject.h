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
		glm::mat4* ExternalMatrix;

		bool Dynamic;

		float Mu;
		float Bounciness;
	};

	PhysicalValues PhysicalParams;

	PhysicalObject()
	{
		PhysicalParams.Enabled = false;
		PhysicalParams.Dynamic = false;
		PhysicalParams.ExternalMatrix = nullptr;
	}

	virtual ~PhysicalObject()
	{ }

	virtual uint32_t RayCastCallback(void* userPointer)
	{
		return 0;
	}
};

#endif
