#ifndef _PROGRAM_H
#define _PROGRAM_H

#include "global.h"

class Program
{
public:
	Program();
	~Program();

	void Run();

private:
	Engine _engine;
};

#endif
