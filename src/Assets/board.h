#ifndef _BOARD_H
#define _BOARD_H

#include "../VideoEngine/video.h"

class Board : public Model
{
public:
	Board(Video* video)
	{
		_video = video;

		Loader::VertexData model;

		model.Vertices = {
			{0+5, 0, 0},
			{1+5, 0, 0},
			{0+5, 0, 1},
			{1+5, 0, 1},
			{0+5, 0, 0},
			{1+5, 0, 0},
			{0+5, 0, 1},
			{1+5, 0, 1}
		};

		model.Normals = {
			{0, -1, 0},
			{0, -1, 0},
			{0, -1, 0},
			{0, -1, 0},
			{0, 1, 0},
			{0, 1, 0},
			{0, 1, 0},
			{0, 1, 0}
		};

		model.TexCoords = {
			{0, 0},
			{1, 0},
			{0, 1},
			{1, 1},
			{0, 0},
			{1, 0},
			{0, 1},
			{1, 1}
		};

		model.Indices = {
			0, 1, 2, 2, 1, 3,
			4, 6, 5, 6, 7, 5
		};

		glm::mat4 instance1 = glm::mat4(1.0f);
		glm::mat4 instance2 = glm::translate(
			glm::mat4(1.0f),
			glm::vec3(0, 0.2, 0));

		model.Instances = {instance1, instance2};

		_model = _video->LoadModel(model);

		SetModels({_model});

		SetModelMatrix(glm::mat4(1.0f));
		SetModelInnerMatrix(glm::mat4(1.0f));

		SetDrawEnabled(true);
	}

	~Board()
	{
		_video->UnloadModel(_model);
	}

private:
	Video* _video;
	uint32_t _model;
};

#endif
