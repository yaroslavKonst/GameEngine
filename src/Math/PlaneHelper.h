#ifndef _PLANE_HELPER_H
#define _PLANE_HELPER_H

#include <cmath>

#include "vec.h"

namespace PlaneHelper
{
	typedef Math::Vec<4> Plane;

	inline Plane PlaneByThreePoints(
		const Math::Vec<3>& p0,
		const Math::Vec<3>& p1,
		const Math::Vec<3>& p2)
	{
		Plane plane;
		// ax + by + cz + d = 0

		// x
		plane[0] = -p1[1] * p0[2]
			+ p2[1] * p0[2]
			+ p0[1] * p1[2]
			- p2[1] * p1[2]
			- p0[1] * p2[2]
			+ p1[1] * p2[2];

		// y
		plane[1] = p1[0] * p0[2]
			- p2[0] * p0[2]
			- p0[0] * p1[2]
			+ p2[0] * p1[2]
			+ p0[0] * p2[2]
			- p1[0] * p2[2];

		// z
		plane[2] = -p1[0] * p0[1]
			+ p2[0] * p0[1]
			+ p0[0] * p1[1]
			- p2[0] * p1[1]
			- p0[0] * p2[1]
			+ p1[0] * p2[1];

		// 1
		plane[3] = p2[0] * p1[1] * p0[2]
			-p1[0] * p2[1] * p0[2]
			-p2[0] * p0[1] * p1[2]
			+p0[0] * p2[1] * p1[2]
			+p1[0] * p0[1] * p2[2]
			-p0[0] * p1[1] * p2[2];

		return plane;
	}

	inline float PointToPlaneDistance(
		const Math::Vec<3>& point,
		const Plane& plane)
	{
		float u = fabsf(plane[0] * point[0] +
			plane[1] * point[1] +
			plane[2] * point[2] +
			plane[3]);

		float d = sqrtf(plane[0] * plane[0] +
			plane[1] * plane[1] +
			plane[2] * plane[2]);

		return u / d;
	}

	inline double SetPointToPlane(
		const Math::Vec<3>& point,
		const Plane& plane)
	{
		return
			point[0] * plane[0] +
			point[1] * plane[1] +
			point[2] * plane[2] + plane[3];
	}

	inline Math::Vec<3> ProjectPointToPlane(
		const Math::Vec<3>& point,
		const Plane& plane)
	{
		double alpha = -SetPointToPlane(point, plane) /
			(plane[0] * plane[0] +
			plane[1] * plane[1] +
			plane[2] * plane[2]);

		return point + Math::Vec<3>(
			{plane[0], plane[1], plane[2]}) * alpha;
	}

	inline float TriangleSqr(const std::vector<Math::Vec<3>>& triangle)
	{
		double l1 = (triangle[1] - triangle[0]).Length();
		double l2 = (triangle[2] - triangle[0]).Length();
		double l3 = (triangle[2] - triangle[1]).Length();

		double h = (l1 + l2 + l3) / 2.0;

		return sqrt(h * (h - l1) * (h - l2) * (h - l3));
	}

	inline bool PointInTriangle(
		const Math::Vec<3>& point,
		const std::vector<Math::Vec<3>>& triangle)
	{
		Plane tPlane = PlaneByThreePoints(
			triangle[0],
			triangle[1],
			triangle[2]);

		Math::Vec<3> outPoint = point + Math::Vec<3>(
			{tPlane[0], tPlane[1], tPlane[2]});

		Plane plane1 = PlaneByThreePoints(
			triangle[0],
			outPoint,
			triangle[2]);
		Plane plane2 = PlaneByThreePoints(
			triangle[0],
			triangle[1],
			outPoint);
		Plane plane3 = PlaneByThreePoints(
			triangle[1],
			triangle[2],
			outPoint);

		double v1 = SetPointToPlane(point, plane1);
		double v2 = SetPointToPlane(point, plane2);
		double v3 = SetPointToPlane(point, plane3);

		return v1 * v2 >= 0 && v1 * v3 >= 0 && v2 * v3 >= 0;
	}

	inline bool RayIntersectSphere(
		const Math::Vec<3>& point,
		const Math::Vec<3>& direction,
		const Math::Vec<3>& center,
		double radius,
		double& res1,
		double& res2)
	{
		double a =
			pow(direction[0], 2) +
			pow(direction[1], 2) +
			pow(direction[2], 2);

		double xf = point[0] - center[0];
		double yf = point[1] - center[1];
		double zf = point[2] - center[2];

		double b = 2.0 * (
			direction[0] * xf +
			direction[1] * yf +
			direction[2] * zf);

		double c = pow(xf, 2) + pow(yf, 2) + pow(zf, 2) -
			pow(radius, 2);

		double D = pow(b, 2) - a * c * 4;

		if (D < 0) {
			return false;
		}

		res1 = (-b - sqrt(D)) / (a * 2);
		res2 = (-b + sqrt(D)) / (a * 2);

		return true;
	}

	inline bool RayIntersectPlane(
		const Math::Vec<3>& point,
		const Math::Vec<3>& direction,
		const Plane& plane,
		double& result)
	{
		double f2 =
			plane[0] * direction[0] +
			plane[1] * direction[1] +
			plane[2] * direction[2];

		if (abs(f2) < std::numeric_limits<double>::epsilon()) {
			return false;
		}

		result = -SetPointToPlane(point, plane) / f2;

		return true;
	}
}

#endif
