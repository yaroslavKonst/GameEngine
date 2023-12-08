#ifndef _PHYSICAL_ENGINE_H
#define _PHYSICAL_ENGINE_H

#include <set>

#include "PhysicalObject.h"
#include "../Utils/ThreadPool.h"

class PhysicalEngine
{
public:
	struct RayCastResult
	{
		PhysicalObject* object;
		uint32_t Code;
	};

	PhysicalEngine();
	~PhysicalEngine();

	void Run(ThreadPool* threadPool);

	void RegisterObject(PhysicalObject* object);
	void RemoveObject(PhysicalObject* object);

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
	};

	std::set<PhysicalObject*> _objects;
	std::map<PhysicalObject*, ObjectDescriptor*> _objectDescriptors;

	std::mutex _mutex;

	void InitializeObject(PhysicalObject* object);
	void DeinitializeObject(PhysicalObject* object);
};

#endif
