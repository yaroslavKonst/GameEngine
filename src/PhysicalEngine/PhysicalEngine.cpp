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

static void ToWorldSpace(
	const glm::mat4& transform,
	const std::vector<glm::vec3>& objectSpace,
	const std::vector<glm::vec3>& worldSpace,
	bool points)
{
	float w = points ? 1.0 : 0.0;

	for (size_t i = 0; i < objectSpace.size(); ++i) {
		worldSpace[i] = transform * glm::vec4(objectSpace[i], w);
	}
}

void PhysicalEngine::InitializeObject(PhysicalObject* object)
{
	if (!object->Physics.Dynamic) {
		ObjectDescriptor* desc = new ObjectDescriptor;

		glm::mat4 transform = object->Physics.Matrix;
		glm::mat4* extMat = object->Physics.ExternalMatrix;

		if (extMat) {
			transform = *extMat * transform;
		}

		desc->Vertices.resize(object->Physics.Vertices.size());
		desc->Normals.resize(object->Physics.Normals.size());

		ToWorldSpace(
			transform,
			object->Physics.Vertices,
			desc->Vertices,
			true);

		ToWorldSpace(
			transform,
			object->Physics.Normals,
			desc->Normals,
			false);

		_objectDescriptors[object] = desc;
	}
}

void PhysicalEngine::DeinitializeObject(PhysicalObject* object)
{
	if (_objectDescriptors.find(object) != _objectDescriptors.end()) {
		delete _objectDescriptors[object];
		_objectDescriptors.erase(object);
	}
}















void PhysicalEngine::Run(ThreadPool* threadPool)
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

void PhysicalEngine::CalculateCollision(
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
