#ifndef _SCENE_DESCRIPTOR_H
#define _SCENE_DESCRIPTOR_H

#include <set>
#include <mutex>

#include "model.h"
#include "ModelDescriptor.h"
#include "rectangle.h"
#include "skybox.h"
#include "light.h"

struct SceneDescriptor
{
	std::map<Model*, ModelDescriptor> Models;
	std::map<Rectangle*, ModelDescriptor> Rectangles;
	std::set<Light*> Lights;
	Skybox skybox;

	double FOV;
	glm::vec3 CameraPosition;
	glm::vec3 CameraDirection;
	glm::vec3 CameraUp;

	std::mutex* SceneMutex;
};

#endif
