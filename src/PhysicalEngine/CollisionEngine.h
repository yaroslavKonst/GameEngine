#ifndef _COLLISION_ENGINE_H
#define _COLLISION_ENGINE_H

#include <set>

#include "object.h"
#include "../Utils/ThreadPool.h"

class CollisionEngine
{
public:
	struct RayCastResult
	{
		Object* object;
		uint32_t Code;
	};

	CollisionEngine();
	~CollisionEngine();

	void Run();

	void RegisterObject(Object* object);
	void RemoveObject(Object* object);

	RayCastResult RayCast(
		const glm::vec3& point,
		const glm::vec3& direction,
		float distance,
		void* userPointer,
		std::set<Object*> ignore);

private:
	std::set<Object*> _objects;

	ThreadPool* _threadPool;

	std::mutex _mutex;

	void InitializeObject(Object* object);
	void CalculateCollision(
		Object* object1,
		Object* object2);

	glm::vec3 CalculateEffect(
		const glm::vec3& center1,
		float radius1,
		const std::vector<glm::vec3>& triangle2,
		const glm::vec3& normal2);

	bool FindRayIntersection(
		Object* object,
		const glm::vec3& point,
		const glm::vec3& direction,
		float distance,
		float& result);
};

#endif
