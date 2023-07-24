#include "CollisionEngine.h"

#include "PlaneHelper.h"

#include "../Logger/logger.h"

using namespace PlaneHelper;

CollisionEngine::CollisionEngine()
{
	_threadPool = new ThreadPool(3);
}

CollisionEngine::~CollisionEngine()
{
	delete _threadPool;
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
		if (!object->_IsObjectInitialized()) {
			InitializeObject(object);
		}

		object->SetObjectEffect(glm::vec3(0.0f));

		objects[i] = object;
		++i;
	}

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

			_threadPool->Enqueue(
				[this,
				&objects,
				matrix1,
				matrix2,
				objIdx1,
				objIdx2]() -> void
			{
				CalculateCollision(
					objects[objIdx1],
					objects[objIdx2],
					matrix1,
					matrix2);
			});
		}
	}

	_threadPool->Wait();
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

	auto& speed1 = object1->GetObjectSpeed();
	auto& speed2 = object2->GetObjectSpeed();

	glm::vec3 effect(0.0f);

	for (uint32_t pIdx1 = 0; pIdx1 < collisionPrimitives1.size(); ++pIdx1) {
		for (
			uint32_t pIdx2 = 0;
			pIdx2 < collisionPrimitives2.size();
			++pIdx2)
		{
			effect += CalculateCollision(
				collisionPrimitives1[pIdx1],
				collisionPrimitives2[pIdx2],
				matrix1,
				matrix2,
				speed1,
				speed2);
		}
	}

	object1->IncObjectEffect(effect);
	object2->IncObjectEffect(-effect);
}

glm::vec3 CollisionEngine::CalculateCollision(
	const Object::CollisionPrimitive& primitive1,
	const Object::CollisionPrimitive& primitive2,
	const glm::mat4& matrix1,
	const glm::mat4& matrix2,
	const glm::vec3& speed1,
	const glm::vec3& speed2)
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

		prim1[i] += speed1;
		prim2[i] += speed2;
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
		pl[0] = SetPointToPlane(prim1[i], planes2[0]);
		pl[1] = SetPointToPlane(prim1[i], planes2[1]);
		pl[2] = SetPointToPlane(prim1[i], planes2[2]);
		pl[3] = SetPointToPlane(prim1[i], planes2[3]);

		bool sameSign =
			pl[0] * pl[1] >= 0 &&
			pl[0] * pl[2] >= 0 &&
			pl[0] * pl[3] >= 0 &&
			pl[1] * pl[2] >= 0 &&
			pl[1] * pl[3] >= 0 &&
			pl[2] * pl[3] >= 0;

		if (!sameSign) {
			continue;
		}

		float pc[4];
		pc[0] = SetPointToPlane(center1, planes2[0]);
		pc[1] = SetPointToPlane(center1, planes2[1]);
		pc[2] = SetPointToPlane(center1, planes2[2]);
		pc[3] = SetPointToPlane(center1, planes2[3]);

		Plane planeDir(0.0f);
		int32_t planeIdx = -1;

		for (uint32_t plIdx = 0; plIdx < 4; ++plIdx) {
			if (pc[plIdx] * pl[plIdx] < 0) {
				planeDir = planes2[plIdx];
				planeIdx = plIdx;
				break;
			}
		}

		if (planeIdx == -1) {
			continue;
		}

		float dist = PointToPlaneDistance(prim1[i], planeDir);
		glm::vec3 dir(planeDir[0], planeDir[1], planeDir[2]);
		dir = glm::normalize(dir * pc[planeIdx]);

		effect += dir * dist;
	}

	for (uint32_t i = 0; i < 4; ++i) {
		float pl[4];
		pl[0] = SetPointToPlane(prim2[i], planes1[0]);
		pl[1] = SetPointToPlane(prim2[i], planes1[1]);
		pl[2] = SetPointToPlane(prim2[i], planes1[2]);
		pl[3] = SetPointToPlane(prim2[i], planes1[3]);

		bool sameSign =
			pl[0] * pl[1] >= 0 &&
			pl[0] * pl[2] >= 0 &&
			pl[0] * pl[3] >= 0 &&
			pl[1] * pl[2] >= 0 &&
			pl[1] * pl[3] >= 0 &&
			pl[2] * pl[3] >= 0;

		if (!sameSign) {
			continue;
		}

		float pc[4];
		pc[0] = SetPointToPlane(center2, planes1[0]);
		pc[1] = SetPointToPlane(center2, planes1[1]);
		pc[2] = SetPointToPlane(center2, planes1[2]);
		pc[3] = SetPointToPlane(center2, planes1[3]);

		Plane planeDir(0.0f);
		int32_t planeIdx = -1;

		for (uint32_t plIdx = 0; plIdx < 4; ++plIdx) {
			if (pc[plIdx] * pl[plIdx] < 0) {
				planeDir = planes1[plIdx];
				planeIdx = plIdx;
				break;
			}
		}

		if (planeIdx == -1) {
			continue;
		}

		float dist = PointToPlaneDistance(prim2[i], planeDir);
		glm::vec3 dir(planeDir[0], planeDir[1], planeDir[2]);
		dir = glm::normalize(dir * pc[planeIdx]);

		effect -= dir * dist;
	}

	return effect;
}
