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
}

#endif
