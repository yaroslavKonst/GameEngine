#ifndef _INNER_MAT_TEST
#define _INNER_MAT_TEST

#include "../VideoEngine/model.h"
#include "../VideoEngine/video.h"
#include "../UniverseEngine/actor.h"
#include "../UniverseEngine/universe.h"
#include "../Utils/loader.h"

class InnerMatTest : public Model, public Actor
{
public:
	InnerMatTest(Video* video, Universe* universe)
	{
		_video = video;
		_universe = universe;

		auto geometry = Loader::LoadModel("Models/Test/Cube.obj");
		auto image = Loader::LoadImage("Models/Test/Cube.png");

		ModelParams.Model = _video->LoadModel(geometry);
		ModelParams.Matrix = Math::Mat<4>(1.0);
		ModelParams.Matrix[2][3] = 2.0;
		ModelParams.Matrix[1][3] = 4.0;
		TextureParams.SetAll(video->LoadTexture(image));
		DrawParams.Enabled = true;

		ModelParams.InnerMatrix = {
			Math::Mat<4>(1.0),
			Math::Mat<4>(1.0),
			Math::Mat<4>(1.0)
		};

		_diff = -1.0;
		_step = 0.01;

		ModelParams.InnerMatrix[1][0][3] = _diff;
		ModelParams.InnerMatrix[2][0][3] = -_diff;

		_video->RegisterModel(this);
		_universe->RegisterActor(this);
	}

	~InnerMatTest()
	{
		_video->RemoveModel(this);
		_universe->RemoveActor(this);
		_video->UnloadModel(ModelParams.Model);
	}

	void Tick() override
	{
		_diff += _step;

		ModelParams.InnerMatrix[1][0][3] = _diff;
		ModelParams.InnerMatrix[2][0][3] = -_diff;

		if (_diff > 1 || _diff < -1) {
			_step *= -1;
		}
	}

private:
	Video* _video;
	Universe* _universe;

	float _diff;
	float _step;
};

#endif
