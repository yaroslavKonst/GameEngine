#include "PhysicalEngine.h"

#include "PlaneHelper.h"

#include "../Logger/logger.h"

using namespace PlaneHelper;

PhysicalEngine::PhysicalEngine()
{
}

PhysicalEngine::~PhysicalEngine()
{
	for (auto& desc : _objectDescriptors) {
		delete desc.second;
	}
}

void PhysicalEngine::RegisterObject(PhysicalObject* object)
{
	_mutex.lock();
	InitializeObject(object);
	_objects.insert(object);
	_mutex.unlock();
}

void PhysicalEngine::RemoveObject(PhysicalObject* object)
{
	_mutex.lock();
	_objects.erase(object);
	DeinitializeObject(object);
	_mutex.unlock();
}

void PhysicalEngine::RegisterObject(SoftObject* object)
{
	_mutex.lock();
	_softObjects.insert(object);
	_mutex.unlock();
}

void PhysicalEngine::RemoveObject(SoftObject* object)
{
	_mutex.lock();
	_softObjects.erase(object);
	_mutex.unlock();
}

static void ToWorldSpace(
	const glm::mat4& transform,
	const std::vector<glm::vec3>& objectSpace,
	std::vector<glm::vec3>& worldSpace,
	bool points)
{
	float w = points ? 1.0 : 0.0;

	for (size_t i = 0; i < objectSpace.size(); ++i) {
		worldSpace[i] = transform * glm::vec4(objectSpace[i], w);
	}
}

void PhysicalEngine::UpdateObjectDescriptor(
	PhysicalObject* object,
	ObjectDescriptor& desc)
{
	glm::mat4 transform = object->PhysicalParams.Matrix;
	glm::mat4* extMat = object->PhysicalParams.ExternalMatrix;

	if (extMat) {
		transform = *extMat * transform;
	}

	desc.Vertices.resize(object->PhysicalParams.Vertices.size());
	desc.Normals.resize(object->PhysicalParams.Normals.size());

	ToWorldSpace(
		transform,
		object->PhysicalParams.Vertices,
		desc.Vertices,
		true);

	ToWorldSpace(
		transform,
		object->PhysicalParams.Normals,
		desc.Normals,
		false);

	glm::vec3 center(0, 0, 0);
	float radius = 0;

	for (
		size_t idx = 0;
		idx < object->PhysicalParams.Vertices.size();
		++idx)
	{
		glm::vec3 vertex = desc.Vertices[idx];
		center += vertex;
	}

	center /= desc.Vertices.size();

	for (
		size_t idx = 0;
		idx < object->PhysicalParams.Vertices.size();
		++idx)
	{
		glm::vec3 vertex = desc.Vertices[idx];
		float dist = glm::length(vertex - center);

		if (dist > radius) {
			radius = dist;
		}
	}

	desc.Center = center;
	desc.Radius = radius;
}

void PhysicalEngine::InitializeObject(PhysicalObject* object)
{
	ObjectDescriptor* desc = new ObjectDescriptor;
	UpdateObjectDescriptor(object, *desc);
	_objectDescriptors[object] = desc;
}

void PhysicalEngine::DeinitializeObject(PhysicalObject* object)
{
	delete _objectDescriptors[object];
	_objectDescriptors.erase(object);
}

void PhysicalEngine::Run(ThreadPool* threadPool, float timeStep)
{
	_mutex.lock();

	for (PhysicalObject* object : _objects) {
		if (object->PhysicalParams.Dynamic) {
			UpdateObjectDescriptor(
				object,
				*_objectDescriptors[object]);
		}
	}

	for (
		auto sObj = _softObjects.begin();
		sObj != _softObjects.end();
		++sObj)
	{
		SoftObject* softObject = *sObj;

		for (
			auto obj = _objects.begin();
			obj != _objects.end();
			++obj)
		{
			PhysicalObject* object = *obj;

			threadPool->Enqueue(
				[this, object, softObject, timeStep]() -> void
				{
					CalculateCollision(
						object,
						softObject,
						timeStep);
				});
		}
	}

	threadPool->WaitAll();

	for (SoftObject* object : _softObjects) {
		threadPool->Enqueue(
			[this, object, timeStep]() -> void
			{
				ApplyEffect(object, timeStep);
			});
	}

	threadPool->WaitAll();

	_contacts.clear();

	_mutex.unlock();
}

static bool FindTriangleIntersection(
	const glm::vec3& source,
	const glm::vec3& direction,
	const glm::vec3& v1,
	const glm::vec3& v2,
	const glm::vec3& v3,
	float& distance)
{
	const float EPSILON = 0.0000001;

	glm::vec3 edge1;
	glm::vec3 edge2;
	glm::vec3 rayVecXe2;
	glm::vec3 s;
	glm::vec3 sXe1;

	float det;
	float invDet;
	float u;
	float v;

	edge1 = v2 - v1;
	edge2 = v3 - v1;

	rayVecXe2 = glm::cross(direction, edge2);
	det = glm::dot(edge1, rayVecXe2);

	if (det > -EPSILON && det < EPSILON) {
		return false;    // This ray is parallel to this triangle.
	}

	invDet = 1.0 / det;
	s = source - v1;
	u = invDet * glm::dot(s, rayVecXe2);

	if (u < 0.0 || u > 1.0) {
		return false;
	}

	sXe1 = glm::cross(s, edge1);
	v = invDet * glm::dot(direction, sXe1);

	if (v < 0.0 || u + v > 1.0) {
		return false;
	}

	float t = invDet * glm::dot(edge2, sXe1);

	if (t > 0.0) {
		distance = t;
		return true;
	}

	return false;
}

static bool FindMeshIntersection(
	const glm::vec3& source,
	const glm::vec3& direction,
	const std::vector<glm::vec3>& vertices,
	const std::vector<glm::vec3>& normals,
	const std::vector<uint32_t>& indices,
	float& distance,
	glm::vec3& outNormal)
{
	bool intersection = false;

	for (size_t index = 0; index < indices.size(); index += 3) {
		float dist;

		glm::vec3 normal =
			normals[index] +
			normals[index + 1] +
			normals[index + 2];

		normal = glm::normalize(normal);

		float dot = glm::dot(direction, normal);
		if (dot > 0) {
			continue;
		}

		bool intersect = FindTriangleIntersection(
			source,
			direction,
			vertices[index],
			vertices[index + 1],
			vertices[index + 2],
			dist);

		if (intersect && dist < distance) {
			distance = dist;
			outNormal = normal;
			intersection = true;
		}
	}

	return intersection;
}

void PhysicalEngine::CalculateCollision(
	PhysicalObject* object,
	SoftObject* softObject,
	float timeStep)
{
	size_t vertexIndex = 0;
	for (auto& vertex : softObject->SoftPhysicsParams.Vertices) {
		float distance = 1.0f;
		glm::vec3 normal;

		ObjectDescriptor& desc = *_objectDescriptors[object];

		bool possibleCollision =
			glm::length(vertex.Position - desc.Center) <=
			desc.Radius + glm::length(vertex.Speed) * 2.0f;

		if (!possibleCollision) {
			continue;
		}

		bool intersect = FindMeshIntersection(
			vertex.Position,
			vertex.Speed * timeStep,
			desc.Vertices,
			desc.Normals,
			object->PhysicalParams.Indices,
			distance,
			normal);

		if (intersect) {
			Contact contact;
			contact.Normal = normal;
			contact.Distance = distance;
			contact.VertexIndex = vertexIndex;

			_effectMutex.lock();

			if (_contacts.find(softObject) == _contacts.end()) {
				_contacts[softObject] = std::vector<Contact>();
			}

			_contacts[softObject].push_back(contact);
			_effectMutex.unlock();
		}

		++vertexIndex;
	}
}

void PhysicalEngine::ApplyEffect(SoftObject* object, float timeStep)
{
	auto& vertices = object->SoftPhysicsParams.Vertices;
	auto& links = object->SoftPhysicsParams.Links;
	std::vector<glm::vec3> forces(vertices.size(), {0, 0, 0});

	for (size_t vertex = 0; vertex < vertices.size(); ++vertex) {
		forces[vertex] += vertices[vertex].Force +
			object->SoftPhysicsParams.Force;
	}

	for (auto& link : links) {
		glm::vec3 delta =
			vertices[link.Index1].Position -
			vertices[link.Index2].Position;

		float deltaL = glm::length(delta) - link.Length;
		glm::vec3 force = glm::normalize(delta) * link.K * deltaL;

		glm::vec3 deltaV =
			vertices[link.Index1].Speed -
			vertices[link.Index2].Speed;

		deltaV = delta * glm::dot(delta, deltaV);

		if (glm::dot(deltaV, delta) > 0) {
			// Link length increasing.
			force += glm::length(deltaV) * link.Friction;
		} else if (glm::dot(deltaV, delta) < 0) {
			// Link length decreasing.
			force -= glm::length(deltaV) * link.Friction;
		}

		forces[link.Index1] -= force;
		forces[link.Index2] += force;
	}

	float minDist = 0.0001;

	for (auto& contact : _contacts[object]) {
		size_t vertexIndex = contact.VertexIndex;
		auto& vertex = vertices[vertexIndex];

		glm::vec3 force = forces[vertexIndex];

		glm::vec3 normalSpeed = contact.Normal *
			glm::dot(contact.Normal, vertex.Speed);
		glm::vec3 tangentSpeed = vertex.Speed - normalSpeed;

		glm::vec3 normalForce = contact.Normal *
			glm::dot(contact.Normal, force);
		glm::vec3 tangentForce = force - normalForce;

		float normalMoveDist = glm::length(normalSpeed) * timeStep;
		float normalSurfaceDist = glm::length(normalSpeed) * timeStep *
			contact.Distance;

		float distToReduce = normalMoveDist -
			normalSurfaceDist + minDist;

		if (distToReduce > 0) {
			vertex.Position += contact.Normal * distToReduce;
		}

		glm::vec3 targetSpeed = -normalSpeed * vertex.Bounciness;

		glm::vec3 normalResponse =
			(targetSpeed - normalSpeed) / timeStep - normalForce;

		glm::vec3 tangentResponse;
		float frictionLimit = vertex.Mu * glm::length(normalResponse);

		if (glm::length(tangentSpeed) > 0.00001) {
			tangentResponse =
				-frictionLimit * glm::normalize(tangentSpeed);
		} else {
			tangentResponse = -tangentForce;

			if (glm::length(tangentResponse) > frictionLimit) {
				tangentResponse =
					glm::normalize(tangentResponse) *
					frictionLimit;
			}
		}

		forces[vertexIndex] += normalResponse + tangentResponse;
	}

	size_t vertexIndex = 0;
	for (auto& vertex : vertices) {
		vertex.Position += vertex.Speed * timeStep;
		vertex.Speed += forces[vertexIndex] * timeStep;

		++vertexIndex;
	}
}

PhysicalEngine::RayCastResult PhysicalEngine::RayCast(
	const glm::vec3& point,
	const glm::vec3& direction,
	float distance,
	void* userPointer,
	std::set<PhysicalObject*> ignore)
{
	_mutex.lock();

	PhysicalObject* closestObject = nullptr;

	for (PhysicalObject* object : _objects) {
		if (ignore.find(object) != ignore.end()) {
			continue;
		}

		ObjectDescriptor& desc = *_objectDescriptors[object];

		bool possibleCollision =
			glm::length(point - desc.Center) <=
			desc.Radius + glm::length(direction) * distance;

		if (!possibleCollision) {
			continue;
		}

		glm::vec3 normal;
		float dist = distance;

		bool intersect = FindMeshIntersection(
			point,
			direction,
			desc.Vertices,
			desc.Normals,
			object->PhysicalParams.Indices,
			dist,
			normal);

		if (intersect) {
			if (dist < distance) {
				distance = dist;
				closestObject = object;
			}
		}
	}

	_mutex.unlock();

	RayCastResult res;
	res.object = closestObject;

	if (closestObject) {
		res.Code = closestObject->RayCastCallback(userPointer);
	}

	return res;
}
