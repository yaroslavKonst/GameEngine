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

	Plane PlaneByThreePoints(
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

	float PointToPlaneDistance(const glm::vec3& point, const Plane& plane)
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

	float SetPointToPlane(const glm::vec3& point, const Plane& plane)
	{
		return
			point[0] * plane[0] +
			point[1] * plane[1] +
			point[2] * plane[2] + plane[3];
	}

	glm::vec3 ProjectPointToPlane(
		const glm::vec3& point,
		const Plane& plane)
	{
		float alpha = -SetPointToPlane(point, plane) /
			(plane[0] * plane[0] +
			plane[1] * plane[1] +
			plane[2] * plane[2]);

		return point + alpha * glm::vec3(plane[0], plane[1], plane[2]);
	}

	float TriangleSqr(const std::vector<glm::vec3>& triangle)
	{
		float l1 = glm::length(triangle[1] - triangle[0]);
		float l2 = glm::length(triangle[2] - triangle[0]);
		float l3 = glm::length(triangle[2] - triangle[1]);

		float h = (l1 + l2 + l3) / 2.0f;

		return sqrtf(h * (h - l1) * (h - l2) * (h - l3));
	}

	bool PointInTriangle(
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
}

#endif
