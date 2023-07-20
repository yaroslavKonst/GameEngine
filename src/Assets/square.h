#ifndef _SQUARE_H
#define _SQUARE_H

#include "../VideoEngine/rectangle.h"
#include "../UniverseEngine/actor.h"

class Square : public Rectangle, public Actor
{
public:
	Square();
	~Square();
	void Tick();

private:
	float _time;
};

#endif
