#ifndef _SCRIPT_HANDLER_H
#define _SCRIPT_HANDLER_H

#include <string>

#include "ExternModel.h"
#include "../VideoEngine/video.h"

// Scene script syntax.
// texture <texture name> <path to image>
// object <path to obj> <texture name> <specular texture name> <x> <y> <z>
//     <rotation z> <rotation x> <rotation y> <light multiplier>

class ScriptHandler
{
public:
	struct Scene
	{
		std::vector<ExternModel*> Models;
		std::vector<uint32_t> Textures;
	};

	static Scene LoadScene(std::string file, Video* video);

private:
};

#endif
