#ifndef _PLANE_HELPER_H
#define _PLANE_HELPER_H

#include <cmath>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace PlaneHelper
{
	typedef glm::vec4 Plane;

	inline Plane PlaneByThreePoints(
		const glm::vec3& p0,
		const glm::vec3& p1,
		const glm::vec3& p2)
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
		const glm::vec3& point,
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

	inline float SetPointToPlane(
		const glm::vec3& point,
		const Plane& plane)
	{
		return
			point[0] * plane[0] +
			point[1] * plane[1] +
			point[2] * plane[2] + plane[3];
	}

	inline glm::vec3 ProjectPointToPlane(
		const glm::vec3& point,
		const Plane& plane)
	{
		float alpha = -SetPointToPlane(point, plane) /
			(plane[0] * plane[0] +
			plane[1] * plane[1] +
			plane[2] * plane[2]);

		return point + alpha * glm::vec3(plane[0], plane[1], plane[2]);
	}

	inline float TriangleSqr(const std::vector<glm::vec3>& triangle)
	{
		float l1 = glm::length(triangle[1] - triangle[0]);
		float l2 = glm::length(triangle[2] - triangle[0]);
		float l3 = glm::length(triangle[2] - triangle[1]);

		float h = (l1 + l2 + l3) / 2.0f;

		return sqrtf(h * (h - l1) * (h - l2) * (h - l3));
	}

	inline bool PointInTriangle(
		const glm::vec3& point,
		const std::vector<glm::vec3>& triangle)
	{
		Plane tPlane = PlaneByThreePoints(
			triangle[0],
			triangle[1],
			triangle[2]);

		glm::vec3 outPoint = point + glm::vec3(
			tPlane[0],
			tPlane[1],
			tPlane[2]);

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

		float v1 = SetPointToPlane(point, plane1);
		float v2 = SetPointToPlane(point, plane2);
		float v3 = SetPointToPlane(point, plane3);

		return v1 * v2 >= 0 && v1 * v3 >= 0 && v2 * v3 >= 0;
	}

	inline bool RayIntersectSphere(
		const glm::vec3& point,
		const glm::vec3& direction,
		const glm::vec3& center,
		float radius,
		float& res1,
		float& res2)
	{
		float a =
			powf(direction.x, 2) +
			powf(direction.y, 2) +
			powf(direction.z, 2);

		float xf = point.x - center.x;
		float yf = point.y - center.y;
		float zf = point.z - center.z;

		float b = 2.0 * (
			direction.x * xf +
			direction.y * yf +
			direction.z * zf);

		float c = powf(xf, 2) + powf(yf, 2) + powf(zf, 2) -
			powf(radius, 2);

		float D = powf(b, 2) - a * c * 4;

		if (D < 0) {
			return false;
		}

		res1 = (-b - sqrtf(D)) / (a * 2);
		res2 = (-b + sqrtf(D)) / (a * 2);

		return true;
	}

	inline bool RayIntersectPlane(
		const glm::vec3& point,
		const glm::vec3& direction,
		const Plane& plane,
		float& result)
	{
		float f2 =
			plane[0] * direction.x +
			plane[1] * direction.y +
			plane[2] * direction.z;

		if (fabs(f2) < std::numeric_limits<float>::epsilon()) {
			return false;
		}

		result = -SetPointToPlane(point, plane) / f2;

		return true;
	}
}

#endif
