#ifndef _PLAYER_MODEL_TEST
#define _PLAYER_MODEL_TEST

#include "../Engine/Video/model.h"
#include "../Engine/Video/video.h"
#include "../Engine/Time/actor.h"
#include "../Engine/Time/TimeEngine.h"
#include "../Engine/Utils/loader.h"
#include "../Engine/Math/transform.h"

class PlayerModelTest : public Model, public Actor
{
public:
	PlayerModelTest(Video* video, TimeEngine* universe)
	{
		_video = video;
		_universe = universe;

		auto geometry = Loader::LoadModel("Models/Player/Player.obj");
		auto image = Loader::LoadImage("Images/White.png");

		ModelParams.Model = _video->LoadModel(geometry);
		ModelParams.Matrix = Math::Translate({-5, -2, 4});
		TextureParams.SetAll(video->LoadTexture(image));
		DrawParams.Enabled = true;

		_video->RegisterModel(this);
		_universe->RegisterActor(this);
	}

	~PlayerModelTest()
	{
		_video->RemoveModel(this);
		_universe->RemoveActor(this);
		_video->UnloadModel(ModelParams.Model);
	}

	void Tick() override
	{
	}

private:
	Video* _video;
	TimeEngine* _universe;
};

#endif
