#ifndef _ACTOR_H
#define _ACTOR_H

class Actor
{
public:
	virtual ~Actor();
	virtual void Tick() = 0;
	virtual void TickEarly()
	{ }
};

#endif
