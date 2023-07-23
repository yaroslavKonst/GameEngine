#ifndef _TETRAEDR_H
#define _TETRAEDR_H

#include <cmath>

#include "../VideoEngine/model.h"
#include "../UniverseEngine/actor.h"
#include "../PhysicalEngine/object.h"
#include "../Utils/loader.h"

#include "../Logger/logger.h"

class Tetraedr : public Model, public Actor, public Object
{
public:
	Tetraedr()
	{
		SetModelMatrix(glm::mat4(1.0f));
		SetObjectMatrix(glm::mat4(1.0f));
		SetModelInnerMatrix(glm::mat4(1.0f));
		SetModelInstances({glm::mat4(1.0f)});
		_speed = 0.05;
		_pos = glm::vec3(0.0f, 0.0f, 1.0f);

		Object::CollisionPrimitive primitive;
		primitive.Vertices[0] = {0.0, 1.0, 0.0};
		primitive.Vertices[1] = {0.5, 0.0, 0.0};
		primitive.Vertices[2] = {-0.5, 0.0, 0.0};
		primitive.Vertices[3] = {0.0, 0.0, 1.0};

		SetModelVertices({
			{0.0, 1.0, 0.0},
			{0.5, 0.0, 0.0},
			{-0.5, 0.0, 0.0},
			{0.0, 0.0, 1.0}
		});

		SetCollisionPrimitives({
			primitive
		});

		SetModelIndices({0,2,1,0,1,3,0,3,2,1,2,3,
				 0,1,2,0,3,1,0,2,3,1,3,2});

		SetModelTexCoords({
			{0.5, 0.5},
			{0.5, 0.5},
			{0.5, 0.5},
			{0.5, 0.5}
		});

		SetModelNormals({
			{0.0, 0.0, 1.0},
			{0.0, 0.0, 1.0},
			{0.0, 0.0, 1.0},
			{0.0, 0.0, 1.0}
		});

		int texWidth;
		int texHeight;
		auto texture = Loader::LoadImage(
			"../src/Assets/Resources/Images/texture.jpg",
			texWidth,
			texHeight);

		SetTexWidth(texWidth);
		SetTexHeight(texHeight);
		SetTexData(texture);
		SetDrawEnabled(true);
	}

	void Tick()
	{
		_pos.z += _speed;

		SetModelMatrix(glm::translate(glm::mat4(1.0f), _pos));
		SetObjectMatrix(glm::translate(glm::mat4(1.0f), _pos));

		_speed -= 0.0005;

		glm::vec3 effect = GetObjectEffect();
		SetObjectEffect(glm::vec3(0.0f));

		if (effect.z != 0) {
			if (effect.z * _speed < 0) {
				if (fabs(_speed) > 0.0000001) {
					_speed = -_speed * 0.7;
				} else {
					_speed = 0;
				}
			}
		}
	}

private:
	glm::vec3 _pos;
	float _speed;
};

#endif
