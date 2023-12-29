#ifndef _SPRITE_DESCRIPTOR_H
#define _SPRITE_DESCRIPTOR_H

#include "glm.h"

struct SpriteDescriptor
{
	alignas(16) glm::mat4 ProjView;
	alignas(16) glm::vec4 TexCoords;
	alignas(16) glm::vec3 SpritePos;
	alignas(16) glm::vec3 CameraPos;
	alignas(16) glm::vec3 SpriteUp;
	alignas(8) glm::vec2 Size;
};

#endif
