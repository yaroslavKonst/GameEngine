#ifndef _SCRIPT_HANDLER_H
#define _SCRIPT_HANDLER_H

#include <string>

#include "animation.h"

// Animation script line syntax.
// point <x> <y> <z> <rotation z> <rotation x> <rotation y> <time point>
// cycle <{0, 1}>
// positions <{1, 2}>

class ScriptHandler
{
public:
	static Animation LoadAnimation(std::string file);

private:
};

#endif
