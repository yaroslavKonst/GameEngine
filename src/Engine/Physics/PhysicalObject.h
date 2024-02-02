#ifndef _PHYSICAL_OBJECT_H
#define _PHYSICAL_OBJECT_H

#include <vector>

#include "../Math/vec.h"
#include "../Math/mat.h"

class PhysicalObject
{
public:
	struct PhysicalValues
	{
		bool Enabled;

		std::vector<Math::Vec<3>> Vertices;
		std::vector<Math::Vec<3>> Normals;
		std::vector<uint32_t> Indices;
		Math::Mat<4> Matrix;
		Math::Mat<4>* ExternalMatrix;

		bool Dynamic;

		double Mu;
		double Bounciness;
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
