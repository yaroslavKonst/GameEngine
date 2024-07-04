#ifndef _DATA_BRIDGE_H
#define _DATA_BRIDGE_H

#include <set>

#include "model.h"
#include "ModelDescriptor.h"
#include "rectangle.h"
#include "skybox.h"
#include "light.h"
#include "TextureHandler.h"
#include "sprite.h"
#include "InputControl.h"
#include "UniformBufferStorage.h"
#include "../Utils/RingBuffer.h"

#define RING_BUFFER_SIZE 1024 * 1024

struct Scene
{
	std::set<Model*> Models;
	std::set<Rectangle*> Rectangles;
	std::set<Light*> Lights;
	std::set<Sprite*> Sprites;

	std::set<const Model*> RemovedModels;

	std::vector<Skybox> skybox;

	double FOV;
	Math::Vec<3> CameraPosition;
	Math::Vec<3> CameraDirection;
	Math::Vec<3> CameraUp;
};

struct SceneContainer
{
	struct ModelData
	{
		Model model;
		const Model* pointer;
	};

	std::vector<ModelData> Models;
	std::vector<Rectangle> Rectangles;
	std::vector<Light> Lights;
	std::vector<Sprite> Sprites;

	std::set<const Model*> RemovedModels;

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

	std::set<uint32_t> DescriptorsToDeleteOnReceive;

	RingBuffer<LoadModelMessage> LoadModelMessages;
	RingBuffer<RemoveModelMessage> RemoveModelMessages;

	Scene StagedScene;
	SceneContainer SubmittedScene;
	SceneContainer DrawnScene;

	TextureHandler* Textures;
	UniformBufferStorage* UniformBuffers;

	Sync::Mutex SceneMutex;
	Sync::Mutex ExtModMutex;

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
		ExtModMutex.Lock();
		SceneMutex.Lock();

		SubmittedScene.FOV = StagedScene.FOV;
		SubmittedScene.CameraPosition = {
			StagedScene.CameraPosition[0],
			StagedScene.CameraPosition[1],
			StagedScene.CameraPosition[2]
		};

		SubmittedScene.CameraDirection = {
			StagedScene.CameraDirection[0],
			StagedScene.CameraDirection[1],
			StagedScene.CameraDirection[2]
		};

		SubmittedScene.CameraUp = {
			StagedScene.CameraUp[0],
			StagedScene.CameraUp[1],
			StagedScene.CameraUp[2]
		};

		SubmittedScene.Models.resize(StagedScene.Models.size());
		SubmittedScene.Rectangles.resize(StagedScene.Rectangles.size());
		SubmittedScene.Lights.resize(StagedScene.Lights.size());
		SubmittedScene.Sprites.resize(StagedScene.Sprites.size());

		size_t idx = 0;

		for (auto model : StagedScene.Models) {
			SubmittedScene.Models[idx] = {*model, model};

			const Math::Mat<4>* extMat =
				SubmittedScene.Models[idx].model.ModelParams.ExternalMatrix;

			if (extMat) {
				SubmittedScene.Models[idx].model.ModelParams.Matrix =
					*extMat *
					SubmittedScene.Models[idx].model.ModelParams.Matrix;
			}

			SubmittedScene.Models[idx].model.ModelParams.InnerMatrix =
				model->ModelParams.InnerMatrix;

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

			Math::Vec<3> spriteOffset =
				StagedScene.CameraPosition -
				SubmittedScene.Sprites[idx].
					SpriteParams.Position;

			spriteOffset = spriteOffset.Normalize();

			SubmittedScene.Sprites[idx].SpriteParams.Position +=
				spriteOffset *
				SubmittedScene.Sprites[idx].SpriteParams.Offset;
			++idx;
		}

		SubmittedScene.skybox = StagedScene.skybox;

		SubmittedScene.RemovedModels = StagedScene.RemovedModels;
		StagedScene.RemovedModels.clear();

		SceneMutex.Unlock();
		ExtModMutex.Unlock();

		inputControl->PollEvents();
	}

	void LoadToDrawn()
	{
		SceneMutex.Lock();
		DrawnScene = SubmittedScene;
		SubmittedScene.RemovedModels.clear();
		SceneMutex.Unlock();

		while (!LoadModelMessages.IsEmpty()) {
			auto msg = LoadModelMessages.Get();

			if (
				DescriptorsToDeleteOnReceive.find(msg.Index) !=
				DescriptorsToDeleteOnReceive.end())
			{
				DeletedModelDescriptors.push_back(
					msg.Descriptor);
				DescriptorsToDeleteOnReceive.erase(msg.Index);
			} else {
				ModelDescriptors[msg.Index] = msg.Descriptor;
			}
		}

		while (!RemoveModelMessages.IsEmpty()) {
			auto msg = RemoveModelMessages.Get();

			if (
				ModelDescriptors.find(msg.Index) ==
				ModelDescriptors.end())
			{
				DescriptorsToDeleteOnReceive.insert(msg.Index);
			} else {
				DeletedModelDescriptors.push_back(
					ModelDescriptors[msg.Index]);
				ModelDescriptors.erase(msg.Index);
			}
		}

		Textures->PollTextureMessages();

		for (const Model* model : DrawnScene.RemovedModels) {
			UniformBuffers->Free(model);
		}

		UniformBuffers->SwitchSet();

		for (auto& model : DrawnScene.Models) {
			UniformBuffers->UpdateBuffer(
				model.model,
				model.pointer);
		}
	}
};

#endif
