#include "CollisionEngine.h"

#include "PlaneHelper.h"

using namespace PlaneHelper;

CollisionEngine::CollisionEngine()
{
}

CollisionEngine::~CollisionEngine()
{
}

void CollisionEngine::RegisterObject(Object* object)
{
	if (!object->_IsObjectInitialized()) {
		InitializeObject(object);
	}

	_objects.insert(object);
}

void CollisionEngine::RemoveObject(Object* object)
{
	_objects.erase(object);
}

void CollisionEngine::Run()
{
	std::vector<Object*> objects(_objects.size());

	size_t i = 0;
	for (auto object : _objects) {
		objects[i] = object;
		++i;
	}

	#pragma omp parallel for schedule(dynamic)
	for (
		int64_t objIdx1 = 0;
		objIdx1 < (int64_t)objects.size();
		++objIdx1)
	{
		glm::vec3 center1 = objects[objIdx1]->_GetObjectCenter();
		float radius1 = objects[objIdx1]->_GetObjectRadius();
		glm::mat4 matrix1 = objects[objIdx1]->GetObjectMatrix();

		glm::vec3 center1World = matrix1 * glm::vec4(center1, 1.0f);

		for (
			size_t objIdx2 = objIdx1 + 1;
			objIdx2 < objects.size();
			++objIdx2)
		{
			glm::vec3 center2 =
				objects[objIdx2]->_GetObjectCenter();
			float radius2 =
				objects[objIdx2]->_GetObjectRadius();
			glm::mat4 matrix2 =
				objects[objIdx2]->GetObjectMatrix();

			glm::vec3 center2World =
				matrix2 * glm::vec4(center2, 1.0f);

			float distance =
				glm::length(center2World - center1World);

			if (distance > radius1 + radius2) {
				continue;
			}

			CalculateCollision(
				objects[objIdx1],
				objects[objIdx2],
				matrix1,
				matrix2);
		}
	}
}

void CollisionEngine::InitializeObject(Object* object)
{
	auto& collisionPrimitives = object->GetCollisionPrimitives();

	float radius = 0;
	glm::vec3 center(0.0f);
	uint32_t vertexCount = 0;

	for (auto& primitive : collisionPrimitives) {
		for (uint32_t i = 0; i < 4; ++i) {
			center += primitive.Vertices[i];
			++vertexCount;
		}
	}

	center /= vertexCount;

	for (auto& primitive : collisionPrimitives) {
		for (uint32_t i = 0; i < 4; ++i) {
			float dist = glm::length(
				primitive.Vertices[i] - center);

			if (dist > radius) {
				radius = dist;
			}
		}
	}

	object->_SetObjectCenter(center);
	object->_SetObjectRadius(radius);
	object->_SetObjectInitialized(true);
}

void CollisionEngine::CalculateCollision(
	Object* object1,
	Object* object2,
	const glm::mat4& matrix1,
	const glm::mat4& matrix2)
{
	auto& collisionPrimitives1 = object1->GetCollisionPrimitives();
	auto& collisionPrimitives2 = object2->GetCollisionPrimitives();

	glm::vec3 effect(0.0f);

	for (uint32_t pIdx1 = 0; pIdx1 < collisionPrimitives1.size(); ++pIdx1) {
		for (
			uint32_t pIdx2 = pIdx1 + 1;
			pIdx2 < collisionPrimitives2.size();
			++pIdx2)
		{
			effect += CalculateCollision(
				collisionPrimitives1[pIdx1],
				collisionPrimitives2[pIdx2],
				matrix1,
				matrix2);
		}
	}

	object1->IncObjectEffect(effect);
	object2->IncObjectEffect(-effect);
}

glm::vec3 CollisionEngine::CalculateCollision(
	const Object::CollisionPrimitive& primitive1,
	const Object::CollisionPrimitive& primitive2,
	const glm::mat4& matrix1,
	const glm::mat4& matrix2)
{
	glm::vec3 prim1[4];
	glm::vec3 prim2[4];

	glm::vec3 center1(0.0f);
	glm::vec3 center2(0.0f);

	for (uint32_t i = 0; i < 4; ++i) {
		prim1[i] = matrix1 * glm::vec4(primitive1.Vertices[i], 1.0f);
		prim2[i] = matrix2 * glm::vec4(primitive2.Vertices[i], 1.0f);

		center1 += prim1[i];
		center2 += prim2[i];
	}

	center1 /= 4;
	center2 /= 4;

	// ax + by + cz + d = 0
	Plane planes1[4];
	Plane planes2[4];

	planes1[0] = PlaneByThreePoints(prim1[0], prim1[2], prim1[1]);
	planes1[1] = PlaneByThreePoints(prim1[0], prim1[1], prim1[3]);
	planes1[2] = PlaneByThreePoints(prim1[0], prim1[3], prim1[2]);
	planes1[3] = PlaneByThreePoints(prim1[1], prim1[2], prim1[3]);

	planes2[0] = PlaneByThreePoints(prim2[0], prim2[2], prim2[1]);
	planes2[1] = PlaneByThreePoints(prim2[0], prim2[1], prim2[3]);
	planes2[2] = PlaneByThreePoints(prim2[0], prim2[3], prim2[2]);
	planes2[3] = PlaneByThreePoints(prim2[1], prim2[2], prim2[3]);

	glm::vec3 effect(0.0f);

	for (uint32_t i = 0; i < 4; ++i) {
		float pl[4];
		pl[0] = prim1[i][0] * planes2[0][0] +
			prim1[i][1] * planes2[0][1] +
			prim1[i][2] * planes2[0][2] +
			planes2[0][3];

		pl[1] = prim1[i][0] * planes2[1][0] +
			prim1[i][1] * planes2[1][1] +
			prim1[i][2] * planes2[1][2] +
			planes2[1][3];

		pl[2] = prim1[i][0] * planes2[2][0] +
			prim1[i][1] * planes2[2][1] +
			prim1[i][2] * planes2[2][2] +
			planes2[2][3];

		pl[3] = prim1[i][0] * planes2[3][0] +
			prim1[i][1] * planes2[3][1] +
			prim1[i][2] * planes2[3][2] +
			planes2[3][3];

		bool sameSign =
			pl[0] * pl[1] > 0 &&
			pl[0] * pl[2] > 0 &&
			pl[0] * pl[3] > 0 &&
			pl[1] * pl[2] > 0 &&
			pl[1] * pl[3] > 0 &&
			pl[2] * pl[3] > 0;

		if (!sameSign) {
			continue;
		}

		float pc[4];
		pc[0] = center1[0] * planes2[0][0] +
			center1[1] * planes2[0][1] +
			center1[2] * planes2[0][2] +
			planes2[0][3];

		pc[1] = center1[0] * planes2[1][0] +
			center1[1] * planes2[1][1] +
			center1[2] * planes2[1][2] +
			planes2[1][3];

		pc[2] = center1[0] * planes2[2][0] +
			center1[1] * planes2[2][1] +
			center1[2] * planes2[2][2] +
			planes2[2][3];

		pc[3] = center1[0] * planes2[3][0] +
			center1[1] * planes2[3][1] +
			center1[2] * planes2[3][2] +
			planes2[3][3];

		Plane planeDir(0.0f);
		uint32_t planeIdx = 0;

		for (uint32_t plIdx = 0; plIdx < 4; ++plIdx) {
			if (pc[i] * pl[i] < 0) {
				planeDir = planes2[i];
				planeIdx = i;
				break;
			}
		}

		float dist = PointToPlaneDistance(prim1[i], planeDir);
		glm::vec3 dir(planeDir[0], planeDir[1], planeDir[2]);
		dir = glm::normalize(dir * pc[planeIdx]);

		effect += dir * dist;
	}

	for (uint32_t i = 0; i < 4; ++i) {
		float pl[4];
		pl[0] = prim2[i][0] * planes1[0][0] +
			prim2[i][1] * planes1[0][1] +
			prim2[i][2] * planes1[0][2] +
			planes1[0][3];

		pl[1] = prim2[i][0] * planes1[1][0] +
			prim2[i][1] * planes1[1][1] +
			prim2[i][2] * planes1[1][2] +
			planes1[1][3];

		pl[2] = prim2[i][0] * planes1[2][0] +
			prim2[i][1] * planes1[2][1] +
			prim2[i][2] * planes1[2][2] +
			planes1[2][3];

		pl[3] = prim2[i][0] * planes1[3][0] +
			prim2[i][1] * planes1[3][1] +
			prim2[i][2] * planes1[3][2] +
			planes1[3][3];

		bool sameSign =
			pl[0] * pl[1] > 0 &&
			pl[0] * pl[2] > 0 &&
			pl[0] * pl[3] > 0 &&
			pl[1] * pl[2] > 0 &&
			pl[1] * pl[3] > 0 &&
			pl[2] * pl[3] > 0;

		if (!sameSign) {
			continue;
		}

		float pc[4];
		pc[0] = center2[0] * planes1[0][0] +
			center2[1] * planes1[0][1] +
			center2[2] * planes1[0][2] +
			planes1[0][3];

		pc[1] = center2[0] * planes1[1][0] +
			center2[1] * planes1[1][1] +
			center2[2] * planes1[1][2] +
			planes1[1][3];

		pc[2] = center2[0] * planes1[2][0] +
			center2[1] * planes1[2][1] +
			center2[2] * planes1[2][2] +
			planes1[2][3];

		pc[3] = center2[0] * planes1[3][0] +
			center2[1] * planes1[3][1] +
			center2[2] * planes1[3][2] +
			planes1[3][3];

		Plane planeDir(0.0f);
		uint32_t planeIdx = 0;

		for (uint32_t plIdx = 0; plIdx < 4; ++plIdx) {
			if (pc[i] * pl[i] < 0) {
				planeDir = planes1[i];
				planeIdx = i;
				break;
			}
		}

		float dist = PointToPlaneDistance(prim1[i], planeDir);
		glm::vec3 dir(planeDir[0], planeDir[1], planeDir[2]);
		dir = glm::normalize(dir * pc[planeIdx]);

		effect -= dir * dist;
	}

	return effect;
}