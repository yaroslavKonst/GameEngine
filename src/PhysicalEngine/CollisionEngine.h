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
		void* userPointer);

private:
	std::set<Object*> _objects;

	ThreadPool* _threadPool;

	std::mutex _mutex;

	void InitializeObject(Object* object);
	void CalculateCollision(
		Object* object1,
		Object* object2,
		const glm::mat4& matrix1,
		const glm::mat4& matrix2);

	glm::vec3 CalculateEffectOnPoint(
		const std::vector<glm::vec3>& triangle,
		const glm::vec3& point,
		const glm::vec3& centerP,
		const glm::vec3& centerT);

	bool FindRayIntersection(
		Object* object,
		const glm::vec3& point,
		const glm::vec3& direction,
		float distance,
		float& result);
};

#endif
