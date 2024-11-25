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

void ArmMatrix(
	Math::Vec<3> basePoint,
	Math::Vec<3> baseDir,
	Math::Vec<3> baseForward,
	double totalLen,
	double baseLen,
	Math::Vec<3> endPoint,
	Math::Vec<3> forward,
	Math::Mat<4>& upMat,
	Math::Mat<4>& downMat)
{
	baseDir = baseDir.Normalize();
	forward = forward.Normalize();
	baseForward = baseForward.Normalize();

	Math::Vec<3> baseEndDir = (endPoint - basePoint).Normalize();

	Math::Vec<3> axis1 = baseDir.Cross(baseEndDir);
	double angle1 = acos(baseDir.Dot(baseEndDir));

	upMat = Math::Rotate(angle1, axis1) *
		Math::Translate(-basePoint);

	baseForward = upMat * Math::Vec<4>(baseForward, 0);

	baseForward -= baseForward.Project(baseEndDir);
	baseForward = baseForward.Normalize();

	forward -= forward.Project(baseEndDir);
	forward = forward.Normalize();

	double angle2 = acos(baseForward.Dot(forward));

	if (fabs(angle2) > 0.000001) {
		Math::Vec<3> axis2 = baseForward.Cross(forward);
		upMat = Math::Rotate(angle2, axis2) * upMat;
	}

	double len = (basePoint - endPoint).Length();
	double endLen = totalLen - baseLen;
	double x = (baseLen * baseLen - endLen * endLen + len * len) /
		(len * 2.0);
	double y = sqrt(baseLen * baseLen - x * x);

	Math::Vec<3> bindPoint = basePoint + baseEndDir * x + forward * y;

	Math::Vec<3> baseBindDir = (bindPoint - basePoint).Normalize();
	Math::Vec<3> bindEndDir = (endPoint - bindPoint).Normalize();

	Math::Vec<3> axis3 = baseBindDir.Cross(baseEndDir).Normalize();
	double angle3 = acos(baseBindDir.Dot(baseEndDir));

	upMat = Math::Rotate(-angle3, axis3) * upMat;

	downMat = upMat;
	upMat = Math::Translate(basePoint) * upMat;

	downMat = Math::Translate(-baseBindDir * baseLen) * downMat;

	Math::Vec<3> axis4 = baseBindDir.Cross(bindEndDir).Normalize();
	double angle4 = acos(baseBindDir.Dot(bindEndDir));

	downMat = Math::Rotate(angle4, axis4) * downMat;
	downMat = Math::Translate(bindPoint) * downMat;
}
