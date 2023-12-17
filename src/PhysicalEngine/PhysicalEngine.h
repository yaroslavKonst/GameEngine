#ifndef _PHYSICAL_ENGINE_H
#define _PHYSICAL_ENGINE_H

#include <set>
#include <map>

#include "PhysicalEngineBase.h"
#include "PhysicalObject.h"
#include "SoftObject.h"
#include "../Utils/ThreadPool.h"

class PhysicalEngine : public PhysicalEngineBase
{
public:
	struct RayCastResult
	{
		PhysicalObject* object;
		uint32_t Code;
	};

	PhysicalEngine();
	~PhysicalEngine();

	void Run(ThreadPool* threadPool, float timeStep) override;

	void RegisterObject(PhysicalObject* object);
	void RemoveObject(PhysicalObject* object);
	void RegisterObject(SoftObject* object);
	void RemoveObject(SoftObject* object);

	RayCastResult RayCast(
		const glm::vec3& point,
		const glm::vec3& direction,
		float distance,
		void* userPointer,
		std::set<PhysicalObject*> ignore = {});

private:
	struct ObjectDescriptor
	{
		std::vector<glm::vec3> Vertices;
		std::vector<glm::vec3> Normals;
		glm::vec3 Center;
		float Radius;
	};

	struct Contact
	{
		glm::vec3 Normal;
		float NormalDistance;
		float Mu;
		float Bounciness;

		size_t VertexIndex;
	};

	std::set<PhysicalObject*> _objects;
	std::map<PhysicalObject*, ObjectDescriptor*> _objectDescriptors;
	std::set<SoftObject*> _softObjects;

	std::mutex _mutex;

	std::map<SoftObject*, std::vector<Contact>> _contacts;
	std::mutex _effectMutex;

	void InitializeObject(PhysicalObject* object);
	void DeinitializeObject(PhysicalObject* object);
	void UpdateObjectDescriptor(
		PhysicalObject* object,
		ObjectDescriptor& desc);

	void CalculateCollision(
		PhysicalObject* object,
		SoftObject* SoftObject,
		float timeStep);
	void ApplyForces(SoftObject* object, float timeStep);
	void ApplyCollision(SoftObject* object, float timeStep);
};

#endif
