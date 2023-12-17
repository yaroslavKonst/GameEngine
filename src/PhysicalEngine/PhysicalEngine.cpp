#include "PhysicalEngine.h"

#include "../Logger/logger.h"

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
		if (!object->PhysicalParams.Enabled) {
			continue;
		}

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

		ApplyForces(softObject, timeStep);

		for (
			auto obj = _objects.begin();
			obj != _objects.end();
			++obj)
		{
			PhysicalObject* object = *obj;

			if (!object->PhysicalParams.Enabled) {
				continue;
			}

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
				ApplyCollision(object, timeStep);
			});
	}

	threadPool->WaitAll();

	_contacts.clear();

	_mutex.unlock();
}

static inline float determinant3(
	const glm::vec3& c0,
	const glm::vec3& c1,
	const glm::vec3& c2)
{
	return
		c0[0] * c1[1] * c2[2] -
		c0[0] * c1[2] * c2[1] -
		c0[1] * c1[0] * c2[2] +
		c0[1] * c1[2] * c2[0] +
		c0[2] * c1[0] * c2[1] -
		c0[2] * c1[1] * c2[0];
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

	glm::vec3 edge1 = v2 - v1;
	glm::vec3 edge2 = v3 - v1;

	// u * edge1 + v * edge2 + v1 = source + t * direction
	// u * edge1 + v * edge2 - t * direction = source - v1

	// b = source - v1
	// u * edge1 + v * edge2 - t * direction = b

	float det = determinant3(edge1, edge2, -direction);
	if (det > -EPSILON && det < EPSILON) {
		return false;
	}

	glm::vec3 b = source - v1;

	float u = determinant3(b, edge2, -direction) / det;
	if (u < 0 || u > 1) {
		return false;
	}

	float v = determinant3(edge1, b, -direction) / det;
	if (v < 0 || u + v > 1) {
		return false;
	}

	float t = determinant3(edge1, edge2, b) / det;
	if (t < 0 || t > distance) {
		return false;
	}

	distance = t;
	return true;
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
		float dist = distance;

		uint32_t index1 = indices[index];
		uint32_t index2 = indices[index + 1];
		uint32_t index3 = indices[index + 2];

		glm::vec3 normal =
			normals[index1] +
			normals[index2] +
			normals[index3];

		normal = glm::normalize(normal);

		float dot = glm::dot(direction, normal);
		if (dot > 0) {
			continue;
		}

		bool intersect = FindTriangleIntersection(
			source,
			direction,
			vertices[index1],
			vertices[index2],
			vertices[index3],
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
		ObjectDescriptor& desc = *_objectDescriptors[object];

		bool possibleCollision =
			glm::length(vertex.Position - desc.Center) <=
			desc.Radius + glm::length(vertex.Speed) * 2.0f;

		if (!possibleCollision) {
			continue;
		}

		float distance = glm::length(vertex.Speed) *
			timeStep * 1.01f;

		if (distance < 0.01) {
			distance = 0.01;
		}

		glm::vec3 normal;

		bool intersect = FindMeshIntersection(
			vertex.Position,
			glm::normalize(vertex.Speed),
			desc.Vertices,
			desc.Normals,
			object->PhysicalParams.Indices,
			distance,
			normal);

		if (intersect) {
			Contact contact;
			contact.Normal = normal;
			contact.NormalDistance = distance *
				fabs(glm::dot(
					normal,
					glm::normalize(vertex.Speed)));
			contact.VertexIndex = vertexIndex;
			contact.Mu = object->PhysicalParams.Mu;
			contact.Bounciness =
				object->PhysicalParams.Bounciness;

			_effectMutex.lock();

			if (
				_contacts.find(softObject) ==
				_contacts.end())
			{
				_contacts[softObject] =
					std::vector<Contact>();
			}

			_contacts[softObject].push_back(contact);
			_effectMutex.unlock();
		}

		++vertexIndex;
	}
}

void PhysicalEngine::ApplyForces(SoftObject* object, float timeStep)
{
	auto& vertices = object->SoftPhysicsParams.Vertices;
	auto& links = object->SoftPhysicsParams.Links;
	std::vector<glm::vec3> forces(vertices.size());

	for (size_t idx = 0; idx < vertices.size(); ++idx) {
		forces[idx] = vertices[idx].Force +
			object->SoftPhysicsParams.Force;
	}

	for (auto& link : links) {
		glm::vec3 delta =
			vertices[link.Index1].Position -
			vertices[link.Index2].Position;

		glm::vec3 normDelta = glm::normalize(delta);

		float deltaL = glm::length(delta) - link.Length;
		glm::vec3 force = normDelta * link.K * deltaL;

		glm::vec3 deltaV =
			vertices[link.Index1].Speed -
			vertices[link.Index2].Speed;

		deltaV = normDelta * glm::dot(normDelta, deltaV);

		if (glm::dot(deltaV, delta) > 0) {
			// Link length increasing.
			force += normDelta *
				glm::length(deltaV) * link.Friction;
		} else if (glm::dot(deltaV, delta) < 0) {
			// Link length decreasing.
			force -= normDelta *
				glm::length(deltaV) * link.Friction;
		}

		forces[link.Index1] -= force;
		forces[link.Index2] += force;
	}

	size_t vertexIndex = 0;
	for (auto& vertex : vertices) {
		vertex.Speed += forces[vertexIndex] / vertex.Mass * timeStep;
		++vertexIndex;
	}
}

void PhysicalEngine::ApplyCollision(SoftObject* object, float timeStep)
{
	auto& vertices = object->SoftPhysicsParams.Vertices;
	std::vector<glm::vec3> forces(vertices.size());

	for (size_t idx = 0; idx < vertices.size(); ++idx) {
		forces[idx] = glm::vec3(0, 0, 0);
	}

	float minDist = 0.005;

	for (auto& contact : _contacts[object]) {
		size_t vertexIndex = contact.VertexIndex;
		auto& vertex = vertices[vertexIndex];

		glm::vec3 normalSpeed = contact.Normal *
			glm::dot(contact.Normal, vertex.Speed);
		glm::vec3 tangentSpeed = vertex.Speed - normalSpeed;

		float normalSurfaceDist = contact.NormalDistance;

		if (normalSurfaceDist < minDist) {
			vertex.Position += contact.Normal *
				(minDist - normalSurfaceDist);
		}

		glm::vec3 targetSpeed = contact.Normal *
			glm::length(normalSpeed) *
			(vertex.Bounciness + contact.Bounciness) / 2.0f;

		glm::vec3 normalResponse =
			(targetSpeed - normalSpeed) * vertex.Mass / timeStep;

		float mu = std::min(vertex.Mu, contact.Mu);

		glm::vec3 tangentResponse(0, 0, 0);
		float frictionLimit = mu * glm::length(normalResponse);

		if (glm::length(tangentSpeed) > 0.00001) {
			float maxTangentForce = glm::length(tangentSpeed) *
				vertex.Mass / timeStep;

			tangentResponse =
				-frictionLimit * glm::normalize(tangentSpeed);

			if (glm::length(tangentResponse) > maxTangentForce) {
				tangentResponse =
					glm::normalize(tangentResponse) *
					maxTangentForce;
			}
		}

		forces[vertexIndex] += normalResponse + tangentResponse;
	}

	size_t vertexIndex = 0;
	for (auto& vertex : vertices) {
		vertex.Speed += forces[vertexIndex] / vertex.Mass * timeStep;
		vertex.Position += vertex.Speed * timeStep;

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
	} else {
		res.Code = 0;
	}

	return res;
}
