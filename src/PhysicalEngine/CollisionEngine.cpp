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

	for (size_t objIdx1 = 0; objIdx1 < objects.size(); ++objIdx1)
	{
		bool object1Dynamic = objects[objIdx1]->IsObjectDynamic();

		glm::vec3 center1 = objects[objIdx1]->GetObjectCenter();
		float radius1 = objects[objIdx1]->_GetObjectRadius();
		glm::mat4 matrix1 = objects[objIdx1]->GetObjectMatrix();

		glm::vec3 center1World = matrix1 * glm::vec4(center1, 1.0f);

		for (
			size_t objIdx2 = objIdx1 + 1;
			objIdx2 < objects.size();
			++objIdx2)
		{
			bool object2Dynamic =
				objects[objIdx2]->IsObjectDynamic();

			if (!(object1Dynamic || object2Dynamic)) {
				continue;
			}

			glm::vec3 center2 =
				objects[objIdx2]->GetObjectCenter();
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
	auto& vertices = object->GetObjectVertices();

	float radius = 0;
	glm::vec3 center = object->GetObjectCenter();

	for (auto& vertex : vertices) {
		float dist = glm::length(vertex - center);

		if (dist > radius) {
			radius = dist;
		}
	}

	object->_SetObjectRadius(radius);
	object->_SetObjectInitialized(true);
}

void CollisionEngine::CalculateCollision(
	Object* object1,
	Object* object2,
	const glm::mat4& matrix1,
	const glm::mat4& matrix2)
{
	auto& vertices1 = object1->GetObjectVertices();
	auto& vertices2 = object2->GetObjectVertices();
	auto& indices1 = object1->GetObjectIndices();
	auto& indices2 = object2->GetObjectIndices();

	auto& center1 = object1->GetObjectCenter();
	auto& center2 = object2->GetObjectCenter();

	auto& speed1 = object1->GetObjectSpeed();
	auto& speed2 = object2->GetObjectSpeed();

	std::vector<glm::vec3> verticesWorld1(vertices1.size());
	std::vector<glm::vec3> verticesWorld2(vertices2.size());

	glm::vec3 center1World = matrix1 * glm::vec4(center1, 1.0f);
	glm::vec3 center2World = matrix2 * glm::vec4(center2, 1.0f);

	for (size_t i = 0; i < vertices1.size(); ++i) {
		verticesWorld1[i] =
			glm::vec3(matrix1 * glm::vec4(vertices1[i], 1.0f)) +
			speed1;
	}

	for (size_t i = 0; i < vertices2.size(); ++i) {
		verticesWorld2[i] =
			glm::vec3(matrix2 * glm::vec4(vertices2[i], 1.0f)) +
			speed2;
	}

	glm::vec3 effect(0.0f);

	for (auto& vertex : verticesWorld1) {
		for (uint32_t index = 0; index < indices2.size(); index += 3) {
			effect += CalculateEffectOnPoint(
				{
					verticesWorld2[indices2[index]],
					verticesWorld2[indices2[index + 1]],
					verticesWorld2[indices2[index + 2]]
				},
				vertex,
				center1World,
				center2World);
		}
	}

	for (auto& vertex : verticesWorld2) {
		for (uint32_t index = 0; index < indices1.size(); index += 3) {
			effect -= CalculateEffectOnPoint(
				{
					verticesWorld1[indices1[index]],
					verticesWorld1[indices1[index + 1]],
					verticesWorld1[indices1[index + 2]]
				},
				vertex,
				center2World,
				center1World);
		}
	}

	object1->IncObjectEffect(effect);
	object2->IncObjectEffect(-effect);
}

glm::vec3 CollisionEngine::CalculateEffectOnPoint(
	const std::vector<glm::vec3>& triangle,
	const glm::vec3& point,
	const glm::vec3& centerP,
	const glm::vec3& centerT)
{
	Plane plane = PlaneByThreePoints(
		triangle[0],
		triangle[1],
		triangle[2]);

	float pointInPlane = SetPointToPlane(point, plane);
	float centerPInPlane = SetPointToPlane(centerP, plane);
	float centerTInPlane = SetPointToPlane(centerT, plane);

	if (centerTInPlane * centerPInPlane > 0) {
		return glm::vec3(0.0f);
	}

	if (pointInPlane * centerPInPlane > 0) {
		return glm::vec3(0.0f);
	}

	glm::vec3 projectedPoint = ProjectPointToPlane(
		point,
		plane);

	if (!PointInTriangle(projectedPoint, triangle)) {
		return glm::vec3(0.0f);
	}

	return projectedPoint - point;
}
