#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include "../VideoEngine/model.h"
#include "../UniverseEngine/actor.h"

class Triangle : public Model, public Actor
{
public:
	Triangle();
	~Triangle();
	void Tick();

private:
	float _angle;
};

#endif
