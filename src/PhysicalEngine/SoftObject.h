#ifndef _SOFT_OBJECT_H
#define _SOFT_OBJECT_H

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

class SoftObject
{
public:
	struct SoftPhysicsValues
	{
		struct Vertex
		{
			float Mass;
			float Mu;
			float Bounciness;

			glm::vec3 Position;
			glm::vec3 Speed;

			glm::vec3 Force;

			Vertex()
			{
				Speed = {0, 0, 0};
				Force = {0, 0, 0};
			}
		};

		struct Link
		{
			size_t Index1;
			size_t Index2;

			float Length;
			float K;
			float Friction;
		};

		std::vector<Vertex> Vertices;
		std::vector<Link> Links;

		glm::vec3 Force;
	};

	SoftPhysicsValues SoftPhysicsParams;

	SoftObject()
	{
		SoftPhysicsParams.Force = {0, 0, 0};
	}
};

#endif
