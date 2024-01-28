#ifndef _DEFORMATION_TEST
#define _DEFORMATION_TEST

#include "../VideoEngine/model.h"
#include "../VideoEngine/video.h"
#include "../UniverseEngine/actor.h"
#include "../UniverseEngine/universe.h"
#include "../Utils/loader.h"
#include "../Math/transform.h"

class DeformationTest : public Model, public Actor
{
public:
	DeformationTest(Video* video, Universe* universe)
	{
		_video = video;
		_universe = universe;

		auto geometry = Loader::LoadModel("Models/Test/Beam.obj");
		auto image = Loader::LoadImage("Models/Test/Beam.png");

		ModelParams.Model = _video->LoadModel(geometry);
		ModelParams.Matrix = Math::Mat<4>(1.0);
		ModelParams.Matrix[2][3] = 2.0;
		ModelParams.Matrix[1][3] = -2.0;
		TextureParams.SetAll(video->LoadTexture(image));
		DrawParams.Enabled = true;

		ModelParams.InnerMatrix = {
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

	~DeformationTest()
	{
		_video->RemoveModel(this);
		_universe->RemoveActor(this);
		_video->UnloadModel(ModelParams.Model);
	}

	void Tick() override
	{
		_diff += _step;

		ModelParams.InnerMatrix[1] =
			Math::Rotate(_diff * 3.0, {0 ,0 , 1}, Math::Degrees);

		if (_diff > 10 || _diff < -10) {
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
