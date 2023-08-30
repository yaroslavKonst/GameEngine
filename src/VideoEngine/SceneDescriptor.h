#ifndef _SCENE_DESCRIPTOR_H
#define _SCENE_DESCRIPTOR_H

#include <set>
#include <mutex>

#include "model.h"
#include "ModelDescriptor.h"
#include "rectangle.h"
#include "skybox.h"
#include "light.h"
#include "TextureHandler.h"
#include "sprite.h"

struct SceneDescriptor
{
	std::map<Model*, ModelDescriptor> Models;
	std::set<Rectangle*> Rectangles;
	std::set<Light*> Lights;
	std::set<Sprite*> Sprites;
	Skybox skybox;

	TextureHandler* Textures;

	double FOV;
	glm::vec3 CameraPosition;
	glm::vec3 CameraDirection;
	glm::vec3 CameraUp;

	std::mutex* SceneMutex;

	std::list<ModelDescriptor> DeletedModelDescriptors;
};

#endif
