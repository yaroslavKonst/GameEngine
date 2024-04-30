#include "arm.h"

#include "../../Engine/Math/transform.h"
#include "../../Engine/Logger/logger.h"

void GetArmMatrix(
	Math::Vec<3> basePoint,
	Math::Vec<3> endPoint,
	double totalLen,
	double baseLen,
	Math::Vec<3> forward,
	Math::Vec<3> zeroDir,
	Math::Mat<4>& upMat,
	Math::Mat<4>& downMat)
{
	zeroDir = zeroDir.Normalize();

	double len = (basePoint - endPoint).Length();
	double endLen = totalLen - baseLen;

	double x = (baseLen * baseLen - endLen * endLen + len * len) /
		(len * 2.0);
	double y = sqrt(baseLen * baseLen - x * x);

	Math::Vec<3> xAxis = (endPoint - basePoint).Normalize();
	Math::Vec<3> yAxis = (forward - xAxis * xAxis.Dot(forward)).Normalize();

	Math::Vec<3> bindPoint = basePoint + xAxis * x + yAxis * y;

	Math::Vec<3> baseDir = (bindPoint - basePoint).Normalize();
	Math::Vec<3> endDir = (endPoint - bindPoint).Normalize();

	Math::Vec<3> baseAxis = baseDir.Cross(zeroDir);
	Math::Vec<3> bindAxis = endDir.Cross(zeroDir);

	double baseAngle = acos(baseDir.Dot(zeroDir));
	double endAngle = acos(endDir.Dot(zeroDir));

	upMat =
		Math::Translate(basePoint) *
		Math::Rotate(-baseAngle, baseAxis) *
		Math::Translate(-basePoint);

	downMat =
		Math::Translate(bindPoint) *
		Math::Rotate(-endAngle, bindAxis) *
		Math::Translate(-(basePoint + zeroDir * baseLen));
}
