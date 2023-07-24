#ifndef _COLLISION_ENGINE_H
#define _COLLISION_ENGINE_H

#include <set>

#include "object.h"
#include "../Utils/ThreadPool.h"

class CollisionEngine
{
public:
	CollisionEngine();
	~CollisionEngine();

	void Run();

	void RegisterObject(Object* object);
	void RemoveObject(Object* object);

private:
	std::set<Object*> _objects;

	ThreadPool* _threadPool;

	void InitializeObject(Object* object);
	void CalculateCollision(
		Object* object1,
		Object* object2,
		const glm::mat4& matrix1,
		const glm::mat4& matrix2);

	glm::vec3 CalculateCollision(
		const Object::CollisionPrimitive& primitive1,
		const Object::CollisionPrimitive& primitive2,
		const glm::mat4& matrix1,
		const glm::mat4& matrix2,
		const glm::vec3& speed1,
		const glm::vec3& speed2);
};

#endif
