#ifndef _SCRIPT_HANDLER_H
#define _SCRIPT_HANDLER_H

#include <string>

#include "ExternModel.h"
#include "animation.h"
#include "../VideoEngine/video.h"

// Scene script line syntax.
// texture <texture name> <path to image>
// object <path to obj> <texture name> <specular texture name> <x> <y> <z>
//     <rotation z> <rotation x> <rotation y> <light multiplier>
// light <type {point, spot}> <r> <g> <b> <x> <y> <z>
//     <direction x> <direction y> <direction z> <angle> <fade angle>
//
// Animation script line syntax.
// point <x> <y> <z> <rotation z> <rotation x> <rotation y> <time point>
// cycle <{0, 1}>
// positions <{1, 2}>

class ScriptHandler
{
public:
	struct Scene
	{
		std::vector<ExternModel*> Models;
		std::vector<uint32_t> Textures;
		std::vector<Light*> Lights;
	};

	static Scene LoadScene(std::string file, Video* video);
	static Animation LoadAnimation(std::string file);

private:
};

#endif
