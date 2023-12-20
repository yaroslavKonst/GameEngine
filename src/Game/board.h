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

		Math::Mat<4> instance1(1.0);
		Math::Mat<4> instance2 = GlmToMat(glm::translate(
			glm::dmat4(1.0),
			glm::dvec3(0, 0.2, 0)));

		model.Instances = {instance1, instance2};

		_model = _video->LoadModel(model);

		ModelParams.Model = _model;

		ModelParams.Matrix = Math::Mat<4>(1.0);
		DrawParams.Enabled = true;
	}

	~Board()
	{
		_video->UnloadModel(_model);
	}

private:
	Video* _video;
	uint32_t _model;

	Math::Mat<4> GlmToMat(const glm::dmat4& mat)
	{
		Math::Mat<4> res;

		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				res[row][col] = mat[col][row];
			}
		}

		return res;
	}
};

#endif
