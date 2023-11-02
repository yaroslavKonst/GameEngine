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

struct Scene
{
	std::set<Model*> Models;
	std::set<Rectangle*> Rectangles;
	std::set<Light*> Lights;
	std::set<Sprite*> Sprites;

	double FOV;
	glm::vec3 CameraPosition;
	glm::vec3 CameraDirection;
	glm::vec3 CameraUp;
};

struct SceneContainer
{
	std::vector<Model> Models;
	std::vector<Rectangle> Rectangles;
	std::vector<Light> Lights;
	std::vector<Sprite> Sprites;

	double FOV;
	glm::vec3 CameraPosition;
	glm::vec3 CameraDirection;
	glm::vec3 CameraUp;
};

struct SceneDescriptor
{
	std::map<uint32_t, ModelDescriptor> ModelDescriptors;
	uint32_t LastModelIndex;
	Skybox skybox;

	Scene StagedScene;
	SceneContainer SubmittedScene;
	SceneContainer DrawnScene;

	TextureHandler* Textures;

	std::mutex SceneMutex;

	std::list<ModelDescriptor> DeletedModelDescriptors;

	SceneDescriptor()
	{
		LastModelIndex = 0;
	}

	void Submit()
	{
		SceneMutex.lock();

		SubmittedScene.Models.resize(StagedScene.Models.size());
		SubmittedScene.Rectangles.resize(StagedScene.Rectangles.size());
		SubmittedScene.Lights.resize(StagedScene.Lights.size());
		SubmittedScene.Sprites.resize(StagedScene.Sprites.size());

		size_t idx = 0;

		for (auto model : StagedScene.Models) {
			SubmittedScene.Models[idx] = *model;

			const glm::mat4* extMat =
				SubmittedScene.Models[idx].GetModelExternalMatrix();

			if (extMat) {
				SubmittedScene.Models[idx].SetModelMatrix(
					*extMat *
					SubmittedScene.Models[idx].GetModelMatrix());
			}

			++idx;
		}

		idx = 0;

		for (auto rectangle : StagedScene.Rectangles) {
			SubmittedScene.Rectangles[idx] = *rectangle;
			++idx;
		}

		idx = 0;

		for (auto light : StagedScene.Lights) {
			SubmittedScene.Lights[idx] = *light;
			++idx;
		}

		idx = 0;

		for (auto sprite : StagedScene.Sprites) {
			SubmittedScene.Sprites[idx] = *sprite;
			++idx;
		}

		SubmittedScene.FOV = StagedScene.FOV;
		SubmittedScene.CameraPosition = StagedScene.CameraPosition;
		SubmittedScene.CameraDirection = StagedScene.CameraDirection;
		SubmittedScene.CameraUp = StagedScene.CameraUp;

		SceneMutex.unlock();
	}

	void LoadToDrawn()
	{
		SceneMutex.lock();
		DrawnScene = SubmittedScene;
		SceneMutex.unlock();
	}
};

#endif
