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

	void Run(ThreadPool* threadPool, double timeStep) override;

	void RegisterObject(PhysicalObject* object);
	void RemoveObject(PhysicalObject* object);
	void RegisterObject(SoftObject* object);
	void RemoveObject(SoftObject* object);

	RayCastResult RayCast(
		const Math::Vec<3>& point,
		const Math::Vec<3>& direction,
		double distance,
		void* userPointer,
		std::set<PhysicalObject*> ignore = {});

private:
	struct ObjectDescriptor
	{
		std::vector<Math::Vec<3>> Vertices;
		std::vector<Math::Vec<3>> Normals;
		Math::Vec<3> Center;
		double Radius;
	};

	struct Contact
	{
		Math::Vec<3> Normal;
		double NormalDistance;
		double Mu;
		double Bounciness;

		size_t VertexIndex;
	};

	std::set<PhysicalObject*> _objects;
	std::map<PhysicalObject*, ObjectDescriptor*> _objectDescriptors;
	std::set<SoftObject*> _softObjects;

	Sync::Mutex _mutex;

	std::map<SoftObject*, std::vector<Contact>> _contacts;
	Sync::Mutex _effectMutex;

	void InitializeObject(PhysicalObject* object);
	void DeinitializeObject(PhysicalObject* object);
	void UpdateObjectDescriptor(
		PhysicalObject* object,
		ObjectDescriptor& desc);

	void CalculateCollision(
		PhysicalObject* object,
		SoftObject* SoftObject,
		double timeStep);
	void ApplyForces(SoftObject* object, double timeStep);
	void ApplyCollision(SoftObject* object, double timeStep);
};

#endif
