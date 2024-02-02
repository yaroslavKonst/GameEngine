#ifndef _MODEL_H
#define _MODEL_H

#include <vector>

#include "../Math/mat.h"
#include "texturable.h"

class Model : public Texturable
{
public:
	struct VideoModelValues
	{
		uint32_t Model;
		bool Holed;

		Math::Mat<4> Matrix;
		std::vector<Math::Mat<4>> InnerMatrix;
		const Math::Mat<4>* ExternalMatrix;

		Math::Vec<3> Center;
	};

	VideoModelValues ModelParams;

	Model()
	{
		ModelParams.Holed = false;
		ModelParams.ExternalMatrix = nullptr;
	}
};

#endif
