#ifndef _PHYSICAL_ENGINE_BASE_H
#define _PHYSICAL_ENGINE_BASE_H

#include "../Utils/ThreadPool.h"

class PhysicalEngineBase
{
public:
	virtual ~PhysicalEngineBase()
	{ }

	virtual void Run(ThreadPool* threadPool, float timeStep) = 0;
};

#endif
