#ifndef _SPRITE_DESCRIPTOR_H
#define _SPRITE_DESCRIPTOR_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

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
