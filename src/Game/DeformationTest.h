#ifndef _DEFORMATION_TEST
#define _DEFORMATION_TEST

#include "../Engine/Video/model.h"
#include "../Engine/Video/video.h"
#include "../Engine/Time/actor.h"
#include "../Engine/Time/TimeEngine.h"
#include "../Engine/Utils/loader.h"
#include "../Engine/Math/transform.h"

class DeformationTest : public Model, public Actor
{
public:
	DeformationTest(Video* video, TimeEngine* universe)
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
		_step = 0.1;

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
			Math::Rotate(_diff, {0, 0, 1}, Math::Degrees);

		if (_diff > 100 || _diff < -100) {
			_step *= -1;
		}
	}

private:
	Video* _video;
	TimeEngine* _universe;

	float _diff;
	float _step;
};

#endif
