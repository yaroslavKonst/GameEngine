#ifndef _DATA_BRIDGE_H
#define _DATA_BRIDGE_H

#include <set>
#include <mutex>

#include "model.h"
#include "ModelDescriptor.h"
#include "rectangle.h"
#include "skybox.h"
#include "light.h"
#include "TextureHandler.h"
#include "sprite.h"
#include "InputControl.h"
#include "../Utils/RingBuffer.h"

#define RING_BUFFER_SIZE 1024 * 1024

struct Scene
{
	std::set<Model*> Models;
	std::set<Rectangle*> Rectangles;
	std::set<Light*> Lights;
	std::set<Sprite*> Sprites;

	std::vector<Skybox> skybox;

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

	std::vector<Skybox> skybox;

	double FOV;
	glm::vec3 CameraPosition;
	glm::vec3 CameraDirection;
	glm::vec3 CameraUp;
};

struct LoadModelMessage
{
	uint32_t Index;
	ModelDescriptor Descriptor;
};

struct RemoveModelMessage
{
	uint32_t Index;
};

struct DataBridge
{
	std::map<uint32_t, ModelDescriptor> ModelDescriptors;
	std::set<uint32_t> UsedModelDescriptors;
	uint32_t LastModelIndex;

	RingBuffer<LoadModelMessage> LoadModelMessages;
	RingBuffer<RemoveModelMessage> RemoveModelMessages;

	Scene StagedScene;
	SceneContainer SubmittedScene;
	SceneContainer DrawnScene;

	TextureHandler* Textures;

	std::mutex SceneMutex;
	std::mutex ExtModMutex;

	std::list<ModelDescriptor> DeletedModelDescriptors;

	InputControl* inputControl;

	DataBridge() :
		LoadModelMessages(RING_BUFFER_SIZE),
		RemoveModelMessages(RING_BUFFER_SIZE)
	{
		LastModelIndex = 0;
	}

	void Submit()
	{
		ExtModMutex.lock();
		SceneMutex.lock();

		SubmittedScene.Models.resize(StagedScene.Models.size());
		SubmittedScene.Rectangles.resize(StagedScene.Rectangles.size());
		SubmittedScene.Lights.resize(StagedScene.Lights.size());
		SubmittedScene.Sprites.resize(StagedScene.Sprites.size());

		size_t idx = 0;

		for (auto model : StagedScene.Models) {
			SubmittedScene.Models[idx] = *model;

			const glm::mat4* extMat =
				SubmittedScene.Models[idx].ModelParams.ExternalMatrix;

			if (extMat) {
				SubmittedScene.Models[idx].ModelParams.Matrix =
					*extMat *
					SubmittedScene.Models[idx].ModelParams.Matrix;
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

		SubmittedScene.skybox = StagedScene.skybox;

		inputControl->SubmitEvents();

		SceneMutex.unlock();
		ExtModMutex.unlock();

		inputControl->InvokeEvents();
	}

	void LoadToDrawn()
	{
		SceneMutex.lock();
		DrawnScene = SubmittedScene;
		inputControl->PollEvents();
		SceneMutex.unlock();

		while (!LoadModelMessages.IsEmpty()) {
			auto msg = LoadModelMessages.Get();

			ModelDescriptors[msg.Index] = msg.Descriptor;
		}

		while (!RemoveModelMessages.IsEmpty()) {
			auto msg = RemoveModelMessages.Get();

			DeletedModelDescriptors.push_back(
				ModelDescriptors[msg.Index]);
			ModelDescriptors.erase(msg.Index);
		}

		Textures->PollTextureMessages();
	}
};

#endif
