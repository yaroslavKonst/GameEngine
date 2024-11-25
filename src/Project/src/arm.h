#ifndef ARM_H
#define ARM_H

#include "../../Engine/Math/vec.h"
#include "../../Engine/Math/mat.h"

void GetArmMatrix(
	Math::Vec<3> basePoint,
	Math::Vec<3> endPoint,
	double totalLen,
	double baseLen,
	Math::Vec<3> forward,
	Math::Vec<3> zeroDir,
	Math::Mat<4>& upMat,
	Math::Mat<4>& downMat);

void ArmMatrix(
	Math::Vec<3> basePoint,
	Math::Vec<3> baseDir,
	Math::Vec<3> baseForward,
	double totalLen,
	double baseLen,
	Math::Vec<3> endPoint,
	Math::Vec<3> forward,
	Math::Mat<4>& upMat,
	Math::Mat<4>& downMat);

#endif
