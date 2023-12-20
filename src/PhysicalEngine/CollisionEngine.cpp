#include "CollisionEngine.h"

#include "../Math/PlaneHelper.h"

#include "../Logger/logger.h"

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

	_mutex.lock();
	_objects.insert(object);
	_mutex.unlock();
}

void CollisionEngine::RemoveObject(Object* object)
{
	_mutex.lock();
	_objects.erase(object);
	_mutex.unlock();
}

void CollisionEngine::Run(ThreadPool* threadPool, double timeStep)
{
	_mutex.lock();
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

		if (!object1Dynamic) {
			continue;
		}

		uint32_t object1Domain = objects[objIdx1]->GetObjectDomain();

		for (
			size_t objIdx2 = 0;
			objIdx2 < objects.size();
			++objIdx2)
		{
			if (objIdx1 == objIdx2) {
				continue;
			}

			uint32_t object2Domain =
				objects[objIdx2]->GetObjectDomain();

			bool uncheckedDomains =
				object1Domain > 0 &&
				object1Domain == object2Domain;

			if (uncheckedDomains) {
				continue;
			}

			threadPool->Enqueue(
				[this,
				&objects,
				objIdx1,
				objIdx2]() -> void
			{
				CalculateCollision(
					objects[objIdx1],
					objects[objIdx2]);
			});
		}
	}

	threadPool->WaitAll();

	_mutex.unlock();
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
	Object* object2)
{
	auto& center1 = object1->GetObjectCenter();
	auto& center2 = object2->GetObjectCenter();

	glm::mat4 matrix1 = object1->GetObjectMatrix();
	glm::mat4 matrix2 = object2->GetObjectMatrix();

	glm::mat4* extMatrix1 = object1->GetObjectExternalMatrix();
	glm::mat4* extMatrix2 = object2->GetObjectExternalMatrix();

	if (extMatrix1) {
		matrix1 = *extMatrix1 * matrix1;
	}

	if (extMatrix2) {
		matrix2 = *extMatrix2 * matrix2;
	}

	float radius1 = object1->_GetObjectRadius();
	float radius2 = object2->_GetObjectRadius();

	glm::vec3 center1World = matrix1 * glm::vec4(center1, 1.0f);
	glm::vec3 center2World = matrix2 * glm::vec4(center2, 1.0f);

	float distance = glm::length(center2World - center1World);

	if (distance > radius1 + radius2) {
		return;
	}

	auto& vertices2 = object2->GetObjectVertices();
	auto& normals2 = object2->GetObjectNormals();
	auto& indices2 = object2->GetObjectIndices();

	std::vector<glm::vec3> verticesWorld2(vertices2.size());
	std::vector<glm::vec3> normalsWorld2(indices2.size() / 3);

	for (size_t i = 0; i < vertices2.size(); ++i) {
		verticesWorld2[i] = matrix2 * glm::vec4(vertices2[i], 1.0f);
	}

	for (size_t index = 0; index < indices2.size(); index += 3) {
		glm::vec3 normal =
			(normals2[indices2[index]] +
			normals2[indices2[index + 1]] +
			normals2[indices2[index + 2]]) / 3.0f;

		normalsWorld2[index / 3] = matrix2 * glm::vec4(normal, 0.0f);
	}

	glm::vec3 sphereCenter1 = matrix1 * glm::vec4(
		object1->GetObjectSphereCenter(),
		1.0f);
	float sphereRadius1 = object1->GetObjectSphereRadius();

	glm::vec3 effect(0.0f);

	for (uint32_t index2 = 0; index2 < indices2.size(); index2 += 3)
	{
		glm::vec3 eff = CalculateEffect(
			sphereCenter1,
			sphereRadius1,
			{
				verticesWorld2[indices2[index2]],
				verticesWorld2[indices2[index2 + 1]],
				verticesWorld2[indices2[index2 + 2]]
			},
			normalsWorld2[index2 / 3]);

		if (glm::length(eff) > glm::length(effect)) {
			effect = eff;
		}
	}

	object1->IncObjectEffect(effect, object2);
}

bool LineEffectOnSphere(
	const glm::vec3& center,
	float radius,
	const glm::vec3& point1,
	const glm::vec3& point2,
	glm::vec3& result)
{
	float alpha1;
	float alpha2;

	bool intersect = RayIntersectSphere(
		point1,
		point2 - point1,
		center,
		radius,
		alpha1,
		alpha2);

	if (!intersect) {
		return false;
	}

	if ((alpha1 < 0 && alpha2 < 0) || (alpha1 > 1 && alpha2 > 1)) {
		return false;
	}

	if ((alpha1 + alpha2) / 2.0 <= 0) {
		result = glm::normalize(center - point1) *
			(radius - glm::length(center - point1));
		return true;
	}

	if ((alpha1 + alpha2) / 2.0 >= 1) {
		result = glm::normalize(center - point2) *
			(radius - glm::length(center - point2));
		return true;
	}

	glm::vec3 point3 = point1 + (point2 - point1) *
		((alpha1 + alpha2) / 2.0f);

	result = glm::normalize(center - point3) *
		(radius - glm::length(center - point3));
	return true;
}

glm::vec3 CollisionEngine::CalculateEffect(
	const glm::vec3& center1,
	float radius1,
	const std::vector<glm::vec3>& triangle2,
	const glm::vec3& normal2)
{
	Plane plane2 = PlaneByThreePoints(
		triangle2[0],
		triangle2[1],
		triangle2[2]);

	float nSign = SetPointToPlane(triangle2[0] + normal2, plane2);
	float sSign = SetPointToPlane(center1, plane2);

	if (nSign * sSign < 0) {
		return glm::vec3(0.0f);
	}

	glm::vec3 projCenter = ProjectPointToPlane(center1, plane2);

	if (PointInTriangle(projCenter, triangle2)) {
		float dist = PointToPlaneDistance(center1, plane2);

		if (dist >= radius1) {
			return glm::vec3(0.0f);
		}

		return glm::normalize(normal2) * (radius1 - dist);
	}

	glm::vec3 result;

	// Edge 1
	if (LineEffectOnSphere(
		center1,
		radius1,
		triangle2[0],
		triangle2[1],
		result))
	{
		return result;
	}

	// Edge 2
	if (LineEffectOnSphere(
		center1,
		radius1,
		triangle2[1],
		triangle2[2],
		result))
	{
		return result;
	}

	// Edge 3
	if (LineEffectOnSphere(
		center1,
		radius1,
		triangle2[0],
		triangle2[2],
		result))
	{
		return result;
	}

	return glm::vec3(0.0f);
}

CollisionEngine::RayCastResult CollisionEngine::RayCast(
	const glm::vec3& point,
	const glm::vec3& direction,
	float distance,
	void* userPointer,
	std::set<Object*> ignore)
{
	glm::vec3 dir = glm::normalize(direction);

	float closestHitDistance = distance;
	Object* closestObject = nullptr;

	_mutex.lock();

	for (auto object : _objects) {
		if (!object->_IsObjectInitialized()) {
			InitializeObject(object);
		}

		if (ignore.find(object) != ignore.end()) {
			continue;
		}

		glm::vec3 center = object->GetObjectCenter();
		float radius = object->_GetObjectRadius();
		glm::mat4 matrix = object->GetObjectMatrix();
		glm::mat4* extMatrix = object->GetObjectExternalMatrix();

		if (extMatrix) {
			matrix = *extMatrix * matrix;
		}

		glm::vec3 centerWorld = matrix * glm::vec4(center, 1.0f);

		float dist = glm::length(centerWorld - point) - radius;

		if (dist > closestHitDistance) {
			continue;
		}

		bool isValid = FindRayIntersection(
			object,
			point,
			dir,
			distance,
			dist);

		if (isValid && dist < closestHitDistance) {
			closestObject = object;
			closestHitDistance = dist;
		}
	}

	_mutex.unlock();

	RayCastResult result;
	result.object = closestObject;
	result.Code = 0;

	if (closestObject) {
		result.Code = closestObject->RayCastCallback(userPointer);
	}

	return result;
}

bool CollisionEngine::FindRayIntersection(
	Object* object,
	const glm::vec3& point,
	const glm::vec3& direction,
	float distance,
	float& result)
{
	glm::vec3 center = object->GetObjectCenter();
	float radius = object->_GetObjectRadius();
	glm::mat4 matrix = object->GetObjectMatrix();
	glm::mat4* extMatrix = object->GetObjectExternalMatrix();

	if (extMatrix) {
		matrix = *extMatrix * matrix;
	}

	glm::vec3 centerWorld = matrix * glm::vec4(center, 1.0f);

	// Sphere: X : length(centerWorld, X) == radius.
	// Ray: point + alpha * normalize(direction) where
	//   0 <= alpha <= distance.
	float p1, p2;

	bool sphereInt = RayIntersectSphere(
		point,
		direction,
		centerWorld,
		radius,
		p1,
		p2);

	if (!sphereInt) {
		return false;
	}

	if (p1 < 0 && p2 < 0) {
		return false;
	}

	if (p1 > distance && p2 > distance) {
		return false;
	}

	auto& vertices = object->GetObjectVertices();
	auto& indices = object->GetObjectIndices();

	std::vector<glm::vec3> verticesWorld(vertices.size());

	for (size_t i = 0; i < vertices.size(); ++i) {
		verticesWorld[i] = matrix * glm::vec4(vertices[i], 1.0f);
	}

	float closestHitDistance = distance;
	bool hitFound = false;

	for (uint32_t index = 0; index < indices.size(); index += 3) {
		uint32_t index1 = indices[index];
		uint32_t index2 = indices[index + 1];
		uint32_t index3 = indices[index + 2];

		Plane plane = PlaneByThreePoints(
			verticesWorld[index1],
			verticesWorld[index2],
			verticesWorld[index3]);

		float dist;

		bool isIntersect = RayIntersectPlane(
			point,
			direction,
			plane,
			dist);

		if (!isIntersect || dist < 0 || dist > closestHitDistance) {
			continue;
		}

		glm::vec3 intersectPoint = point + direction * dist;

		if (!PointInTriangle(
			intersectPoint,
			{
				verticesWorld[index1],
				verticesWorld[index2],
				verticesWorld[index3]
			}))
		{
			continue;
		}

		if (dist < closestHitDistance) {
			closestHitDistance = dist;
			hitFound = true;
		}
	}

	if (hitFound) {
		result = closestHitDistance;
		return true;
	}

	return false;
}
