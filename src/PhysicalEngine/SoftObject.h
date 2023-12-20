#ifndef _SOFT_OBJECT_H
#define _SOFT_OBJECT_H

#include <vector>

#include "../Math/vec.h"
#include "../Math/mat.h"

class SoftObject
{
public:
	struct SoftPhysicsValues
	{
		struct Vertex
		{
			double Mass;
			double Mu;
			double Bounciness;

			Math::Vec<3> Position;
			Math::Vec<3> Speed;

			Math::Vec<3> Force;

			Vertex()
			{
				Speed = Math::Vec<3>(0.0);
				Force = Math::Vec<3>(0.0);
			}
		};

		struct Link
		{
			size_t Index1;
			size_t Index2;

			double Length;
			double K;
			double Friction;
		};

		std::vector<Vertex> Vertices;
		std::vector<Link> Links;

		Math::Vec<3> Force;
	};

	SoftPhysicsValues SoftPhysicsParams;

	SoftObject()
	{
		SoftPhysicsParams.Force = Math::Vec<3>(0.0);
	}
};

#endif
