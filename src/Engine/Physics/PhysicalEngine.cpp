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
	_mutex.Lock();
	InitializeObject(object);
	_objects.insert(object);
	_mutex.Unlock();
}

void PhysicalEngine::RemoveObject(PhysicalObject* object)
{
	_mutex.Lock();
	_objects.erase(object);
	DeinitializeObject(object);
	_mutex.Unlock();
}

void PhysicalEngine::RegisterObject(SoftObject* object)
{
	_mutex.Lock();
	_softObjects.insert(object);
	_mutex.Unlock();
}

void PhysicalEngine::RemoveObject(SoftObject* object)
{
	_mutex.Lock();
	_softObjects.erase(object);
	_mutex.Unlock();
}

static void ToWorldSpace(
	const Math::Mat<4>& transform,
	const std::vector<Math::Vec<3>>& objectSpace,
	std::vector<Math::Vec<3>>& worldSpace,
	bool points)
{
	double w = points ? 1.0 : 0.0;

	for (size_t i = 0; i < objectSpace.size(); ++i) {
		worldSpace[i] = transform * Math::Vec<4>(objectSpace[i], w);
	}
}

void PhysicalEngine::UpdateObjectDescriptor(
	PhysicalObject* object,
	ObjectDescriptor& desc)
{
	Math::Mat<4> transform = object->PhysicalParams.Matrix;
	Math::Mat<4>* extMat = object->PhysicalParams.ExternalMatrix;

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

	Math::Vec<3> center(0.0);
	double radius = 0;

	for (
		size_t idx = 0;
		idx < object->PhysicalParams.Vertices.size();
		++idx)
	{
		Math::Vec<3> vertex = desc.Vertices[idx];
		center += vertex;
	}

	center /= desc.Vertices.size();

	for (
		size_t idx = 0;
		idx < object->PhysicalParams.Vertices.size();
		++idx)
	{
		Math::Vec<3> vertex = desc.Vertices[idx];
		double dist = (vertex - center).Length();

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

void PhysicalEngine::Run(ThreadPool* threadPool, double timeStep)
{
	_mutex.Lock();

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

	_mutex.Unlock();
}

static inline double determinant3(
	const Math::Vec<3>& c0,
	const Math::Vec<3>& c1,
	const Math::Vec<3>& c2)
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
	const Math::Vec<3>& source,
	const Math::Vec<3>& direction,
	const Math::Vec<3>& v1,
	const Math::Vec<3>& v2,
	const Math::Vec<3>& v3,
	double& distance)
{
	const double EPSILON = 0.00000001;

	Math::Vec<3> edge1 = v2 - v1;
	Math::Vec<3> edge2 = v3 - v1;

	// u * edge1 + v * edge2 + v1 = source + t * direction
	// u * edge1 + v * edge2 - t * direction = source - v1

	// b = source - v1
	// u * edge1 + v * edge2 - t * direction = b

	double det = determinant3(edge1, edge2, -direction);
	if (det > -EPSILON && det < EPSILON) {
		return false;
	}

	Math::Vec<3> b = source - v1;

	double u = determinant3(b, edge2, -direction) / det;
	if (u < 0 || u > 1) {
		return false;
	}

	double v = determinant3(edge1, b, -direction) / det;
	if (v < 0 || u + v > 1) {
		return false;
	}

	double t = determinant3(edge1, edge2, b) / det;
	if (t < 0 || t > distance) {
		return false;
	}

	distance = t;
	return true;
}

static bool FindMeshIntersection(
	const Math::Vec<3>& source,
	const Math::Vec<3>& direction,
	const std::vector<Math::Vec<3>>& vertices,
	const std::vector<Math::Vec<3>>& normals,
	const std::vector<uint32_t>& indices,
	double& distance,
	Math::Vec<3>& outNormal)
{
	bool intersection = false;

	for (size_t index = 0; index < indices.size(); index += 3) {
		double dist = distance;

		uint32_t index1 = indices[index];
		uint32_t index2 = indices[index + 1];
		uint32_t index3 = indices[index + 2];

		Math::Vec<3> normal =
			normals[index1] +
			normals[index2] +
			normals[index3];

		normal = normal.Normalize();

		double dot = direction.Dot(normal);
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
	double timeStep)
{
	size_t vertexIndex = 0;
	for (auto& vertex : softObject->SoftPhysicsParams.Vertices) {
		ObjectDescriptor& desc = *_objectDescriptors[object];

		bool possibleCollision =
			(vertex.Position - desc.Center).Length() <=
			desc.Radius + (vertex.Speed).Length() * 2.0;

		if (!possibleCollision) {
			continue;
		}

		double distance = vertex.Speed.Length() *
			timeStep * 1.01;

		if (distance < 0.01) {
			distance = 0.01;
		}

		Math::Vec<3> normal;

		bool intersect = FindMeshIntersection(
			vertex.Position,
			vertex.Speed.Normalize(),
			desc.Vertices,
			desc.Normals,
			object->PhysicalParams.Indices,
			distance,
			normal);

		if (intersect) {
			Contact contact;
			contact.Normal = normal;
			contact.NormalDistance = distance *
				fabs(normal.Dot(vertex.Speed.Normalize()));
			contact.VertexIndex = vertexIndex;
			contact.Mu = object->PhysicalParams.Mu;
			contact.Bounciness =
				object->PhysicalParams.Bounciness;

			_effectMutex.Lock();

			if (
				_contacts.find(softObject) ==
				_contacts.end())
			{
				_contacts[softObject] =
					std::vector<Contact>();
			}

			_contacts[softObject].push_back(contact);
			_effectMutex.Unlock();
		}

		++vertexIndex;
	}
}

void PhysicalEngine::ApplyForces(SoftObject* object, double timeStep)
{
	auto& vertices = object->SoftPhysicsParams.Vertices;
	auto& links = object->SoftPhysicsParams.Links;
	std::vector<Math::Vec<3>> forces(vertices.size());

	for (size_t idx = 0; idx < vertices.size(); ++idx) {
		forces[idx] = vertices[idx].Force +
			object->SoftPhysicsParams.Force;
	}

	for (auto& link : links) {
		Math::Vec<3> delta =
			vertices[link.Index1].Position -
			vertices[link.Index2].Position;

		Math::Vec<3> normDelta = delta.Normalize();

		double deltaL = delta.Length() - link.Length;
		Math::Vec<3> force = normDelta * link.K * deltaL;

		Math::Vec<3> deltaV =
			vertices[link.Index1].Speed -
			vertices[link.Index2].Speed;

		deltaV = normDelta * normDelta.Dot(deltaV);

		if (deltaV.Dot(delta) > 0) {
			// Link length increasing.
			force += normDelta *
				deltaV.Length() * link.Friction;
		} else if (deltaV.Dot(delta) < 0) {
			// Link length decreasing.
			force -= normDelta *
				deltaV.Length() * link.Friction;
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

void PhysicalEngine::ApplyCollision(SoftObject* object, double timeStep)
{
	auto& vertices = object->SoftPhysicsParams.Vertices;
	std::vector<Math::Vec<3>> forces(vertices.size());

	for (size_t idx = 0; idx < vertices.size(); ++idx) {
		forces[idx] = Math::Vec<3>(0.0);
	}

	double minDist = 0.005;

	for (auto& contact : _contacts[object]) {
		size_t vertexIndex = contact.VertexIndex;
		auto& vertex = vertices[vertexIndex];

		Math::Vec<3> normalSpeed = contact.Normal *
			contact.Normal.Dot(vertex.Speed);
		Math::Vec<3> tangentSpeed = vertex.Speed - normalSpeed;

		double normalSurfaceDist = contact.NormalDistance;

		if (normalSurfaceDist < minDist) {
			vertex.Position += contact.Normal *
				(minDist - normalSurfaceDist);
		}

		Math::Vec<3> targetSpeed = contact.Normal *
			normalSpeed.Length() *
			(vertex.Bounciness + contact.Bounciness) / 2.0;

		Math::Vec<3> normalResponse =
			(targetSpeed - normalSpeed) * vertex.Mass / timeStep;

		double mu = std::min(vertex.Mu, contact.Mu);

		Math::Vec<3> tangentResponse(0.0);
		double frictionLimit = mu * normalResponse.Length();

		if (tangentSpeed.Length() > 0.00001) {
			double maxTangentForce = tangentSpeed.Length() *
				vertex.Mass / timeStep;

			tangentResponse =
				tangentSpeed.Normalize() * -frictionLimit;

			if (tangentResponse.Length() > maxTangentForce) {
				tangentResponse =
					tangentResponse.Normalize() *
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
	const Math::Vec<3>& point,
	const Math::Vec<3>& direction,
	double distance,
	void* userPointer,
	std::set<PhysicalObject*> ignore)
{
	_mutex.Lock();

	PhysicalObject* closestObject = nullptr;

	for (PhysicalObject* object : _objects) {
		if (ignore.find(object) != ignore.end()) {
			continue;
		}

		ObjectDescriptor& desc = *_objectDescriptors[object];

		bool possibleCollision =
			(point - desc.Center).Length() <=
			desc.Radius + direction.Length() * distance;

		if (!possibleCollision) {
			continue;
		}

		Math::Vec<3> normal;
		double dist = distance;

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

	_mutex.Unlock();

	RayCastResult res;
	res.object = closestObject;

	if (closestObject) {
		res.Code = closestObject->RayCastCallback(userPointer);
	} else {
		res.Code = 0;
	}

	return res;
}
