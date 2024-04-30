#include "player.h"

#include "../../Engine/Math/vec.h"
#include "../../Engine/Math/transform.h"
#include "../../Engine/Logger/logger.h"
#include "arm.h"

Player::Player(Engine* engine, GameGlobal* gameGlobal)
{
	_engine = engine;
	_gameGlobal = gameGlobal;

	_x = 0;
	_y = 0;
	_angleH = 0;
	_angleV = 0;

	_go = 0;
	_strafe = 0;

	_legTime = 0;
	_legStep = 0.1;

	_debugAngle = 0;

	_cameraDist = 4.0;

	SetInputLayer(9);

	auto modelData = Loader::LoadModel("Player/Player.obj");
	ModelParams.Model = _engine->video->LoadModel(modelData);

	ModelParams.InnerMatrix = {
		Math::Mat<4>(1.0),
		Math::Mat<4>(1.0),
		Math::Mat<4>(1.0),
		Math::Mat<4>(1.0),
		Math::Mat<4>(1.0),
		Math::Mat<4>(1.0),
		Math::Mat<4>(1.0),
		Math::Mat<4>(1.0),
		Math::Mat<4>(1.0)
	};

	auto textureData = Loader::LoadImage("Player/Player.png");
	TextureParams.SetAll(_engine->video->LoadTexture(textureData));

	DrawParams.Enabled = true;

	_engine->video->Subscribe(this);
	_engine->video->RegisterModel(this);
	_engine->universe->RegisterActor(this);

	SetInputEnabled(true);

	for (int i = 0; i < 4; ++i) {
		_light[i] = new Light();
		_light[i]->Type = Light::Type::Point;
		_light[i]->Color = {10, 10, 10};
		_light[i]->Position = {_x, _y, 10};
		_light[i]->Enabled = true;
		_engine->video->RegisterLight(_light[i]);
	}
}

Player::~Player()
{
	_engine->video->Unsubscribe(this);
	_engine->video->RemoveModel(this);
	_engine->universe->RemoveActor(this);

	for (int i = 0; i < 4; ++i) {
		_engine->video->RemoveLight(_light[i]);
		delete _light[i];
	}

	_engine->video->UnloadTexture(TextureParams.Diffuse);
	_engine->video->UnloadModel(ModelParams.Model);
}

void Player::Tick(double time)
{
	if (_gameGlobal->Paused) {
		return;
	}

	Math::Mat<4> modelMatrix =
		Math::Translate({_x, _y, 1.37}) *
		Math::Rotate(_angleH, {0, 0, 1});

	Math::Vec<2> speed({_go, _strafe});

	ModelParams.Matrix = modelMatrix;

	speed = modelMatrix * Math::Vec<4>(Math::Vec<3>(speed, 0.0), 0.0);

	Math::Vec<3> cameraDirection;
	cameraDirection[0] = cos(_angleH) * cos(_angleV);
	cameraDirection[1] = sin(_angleH) * cos(_angleV);
	cameraDirection[2] = sin(_angleV);

	Math::Vec<3> cameraPosition = {_x, _y, 1.6};
	cameraPosition -= cameraDirection * _cameraDist;

	_engine->video->SetCameraPosition(cameraPosition);
	_engine->video->SetCameraDirection(cameraDirection);

	_light[0]->Position = {_x - 5, _y, 10};
	_light[1]->Position = {_x + 5, _y, 10};
	_light[2]->Position = {_x, _y - 5, 10};
	_light[3]->Position = {_x, _y + 5, 10};

	_x += speed[0] * time;
	_y += speed[1] * time;

	double angle = sin(_legTime) * M_PI / 4;

	ModelParams.InnerMatrix[1] =
		Math::Translate({0, -0.58, 1.48}) *
		Math::Rotate(angle / 3, {0, 1, 0}) *
		Math::Rotate(M_PI / 2.0, {1, 0, 0}) *
		Math::Translate({0, 0.58, -1.48});

	ModelParams.InnerMatrix[2] =
		Math::Translate({0, 0.58, 1.48}) *
		Math::Rotate(angle / 3, {0, -1, 0}) *
		Math::Rotate(M_PI / 2.0, {-1, 0, 0}) *
		Math::Translate({0, -0.58, -1.48});

	Math::Vec<3> leg1End = {
		sin(_legTime) * 0.27,
		-0.2,
		std::max(-1.37 + cos(_legTime) * 0.4, -1.37)
	};

	Math::Vec<3> leg2End = {
		sin(_legTime + M_PI) * 0.27,
		0.2,
		std::max(-1.37 + cos(_legTime + M_PI) * 0.4, -1.37)
	};

	GetArmMatrix(
		{0, -0.2, 0},
		leg1End,
		1.4,
		0.65,
		{1, 0, 0},
		{0, 0, -1},
		ModelParams.InnerMatrix[3],
		ModelParams.InnerMatrix[5]);

	GetArmMatrix(
		{0, 0.2, 0},
		leg2End,
		1.4,
		0.65,
		{1, 0, 0},
		{0, 0, -1},
		ModelParams.InnerMatrix[4],
		ModelParams.InnerMatrix[6]);

	ModelParams.InnerMatrix[7] = Math::Translate(
		leg1End - Math::Vec<3>({0, -0.2, -1.4}));
	ModelParams.InnerMatrix[8] = Math::Translate(
		leg2End - Math::Vec<3>({0, 0.2, -1.4}));

	if (speed.Length() > 0) {
		_legTime += _legStep;
	} else {
		if (_legTime > 0) {
			if (_legTime > M_PI) {
				_legTime += _legStep;
			} else {
				_legTime -= _legStep;
			}
		}
	}

	if (_legTime >= M_PI * 2.0 || _legTime < 0) {
		_legTime = 0;
	}

	_debugAngle += 0.01;

	if (_debugAngle >= M_PI * 2.0) {
		_debugAngle = 0;
	}
}

void Player::Key(int key, int scancode, int action, int mods)
{
	if (_gameGlobal->Paused) {
		return;
	}

	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) {
			_go += 1;
		} else if (action == GLFW_RELEASE) {
			_go -= 1;
		}
	} else if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) {
			_go -= 1;
		} else if (action == GLFW_RELEASE) {
			_go += 1;
		}
	} else if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			_strafe -= 1;
		} else if (action == GLFW_RELEASE) {
			_strafe += 1;
		}
	} else if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			_strafe += 1;
		} else if (action == GLFW_RELEASE) {
			_strafe -= 1;
		}
	}
}

bool Player::MouseMoveRaw(double xoffset, double yoffset)
{
	if (_gameGlobal->Paused) {
		return false;
	}

	_angleH -= xoffset / 100.0;
	_angleV += yoffset / 100.0;

	if (_angleH < 0) {
		_angleH += M_PI * 2.0;
	} else if (_angleH >= M_PI * 2.0) {
		_angleH -= M_PI * 2.0;
	}

	_angleV = std::clamp(_angleV, -M_PI / 2.01, M_PI / 2.01);

	return true;
}

bool Player::Scroll(double xoffset, double yoffset)
{
	if (_gameGlobal->Paused) {
		return false;
	}

	_cameraDist += yoffset * 0.1;

	return true;
}
