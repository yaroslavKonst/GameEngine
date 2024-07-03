#include "player.h"

#include "../../Engine/Math/vec.h"
#include "../../Engine/Math/transform.h"
#include "../../Engine/Logger/logger.h"
#include "arm.h"
#include "surface.h"

Player::Player(
	Engine* engine,
	GameGlobal* gameGlobal,
	std::function<void(double, double)> positionCallback)
{
	_engine = engine;
	_gameGlobal = gameGlobal;
	_positionCallback = positionCallback;

	_x = 0;
	_y = 0;
	_z = 0;
	_angleH = 0;
	_tAngleH = 0;
	_roll = 0;
	_camAngleH = 0;
	_camAngleV = -M_PI / 4.0;
	_block = false;

	_go = 0;
	_strafe = 0;
	_sprint = false;
	_dash = 0;
	_inDash = false;
	_jump = false;

	_legTime = 0;
	_legStep = 0.2;

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

	for (int i = 0; i < 2; ++i) {
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

	for (int i = 0; i < 2; ++i) {
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

	double surfaceHeight = Surface::Height(_x, _y);

	if (_inDash || _dash) {
		ProcessDash(surfaceHeight);
	} else if (!_go && !_strafe) {
		ProcessIdle(surfaceHeight);
	} else if (_sprint) {
		ProcessRun(surfaceHeight);
	} else {
		ProcessWalk(surfaceHeight);
	}

	ProcessBlock();

	SetAngleH();
	SetCameraParams();

	ModelParams.Matrix =
		Math::Translate({_x, _y, _z}) *
		Math::Rotate(_angleH, {0, 0, 1}) *
		Math::Rotate(_roll, {0, 1, 0});

	_light[0]->Position = {_x - 5, _y, 10.0 + _z};
	_light[1]->Position = {_x + 5, _y, 10.0 + _z};

	_positionCallback(_x, _y);

	double delta = 0.001;

	Math::Vec<2> grad = {
		Surface::Height(_x + delta, _y) - surfaceHeight,
		Surface::Height(_x, _y + delta) - surfaceHeight
	};

	grad /= delta;

	if (grad.Length() > 0.8) {
		if (grad.Length() > 2.0) {
			if (grad.Dot({_speed[0], _speed[1]}) > 0) {
				Math::Vec<2> diff = grad.Normalize() *
					grad.Normalize().Dot(
						{_speed[0], _speed[1]});

				_speed[0] -= diff[0];
				_speed[1] -= diff[1];
			}
		}

		_speed[0] -= grad[0];
		_speed[1] -= grad[1];
	}

	_x += _speed[0] * time;
	_y += _speed[1] * time;

	GetArmMatrix(
		{0, -0.58, 1.48},
		_rArm,
		1.42,
		0.77,
		_rArmDir,
		{0, -1, 0},
		ModelParams.InnerMatrix[1],
		ModelParams.InnerMatrix[3]);

	GetArmMatrix(
		{0, 0.58, 1.48},
		_lArm,
		1.42,
		0.77,
		_lArmDir,
		{0, 1, 0},
		ModelParams.InnerMatrix[2],
		ModelParams.InnerMatrix[4]);

	GetArmMatrix(
		{0, -0.2, 0},
		_rLeg,
		1.4,
		0.65,
		{1, 0, 0},
		{0, 0, -1},
		ModelParams.InnerMatrix[5],
		ModelParams.InnerMatrix[7]);

	GetArmMatrix(
		{0, 0.2, 0},
		_lLeg,
		1.4,
		0.65,
		{1, 0, 0},
		{0, 0, -1},
		ModelParams.InnerMatrix[6],
		ModelParams.InnerMatrix[8]);

	ModelParams.InnerMatrix[9] =
		Math::Translate(_rLeg) *
		Math::Rotate(
			std::clamp(-_roll, -M_PI / 4.0, M_PI / 4.0),
			{0, 1, 0}) *
		Math::Translate(-Math::Vec<3>({0, -0.2, -1.4}));
	ModelParams.InnerMatrix[10] =
		Math::Translate(_lLeg) *
		Math::Rotate(
			std::clamp(-_roll, -M_PI / 4.0, M_PI / 4.0),
			{0, 1, 0}) *
		Math::Translate(-Math::Vec<3>({0, 0.2, -1.4}));
}

void Player::SetCameraParams()
{
	Math::Vec<3> cameraDirection;
	cameraDirection[0] = cos(_camAngleH) * cos(_camAngleV);
	cameraDirection[1] = sin(_camAngleH) * cos(_camAngleV);
	cameraDirection[2] = sin(_camAngleV);

	Math::Vec<3> cameraPosition = {_x, _y, _z + 1.0};
	cameraPosition -= cameraDirection * _cameraDist;

	_engine->video->SetCameraPosition(cameraPosition);
	_engine->video->SetCameraDirection(cameraDirection);
}

void Player::SetAngleH()
{
	double step = 0.1;

	double diff = _tAngleH - _angleH;

	if (fabs(diff + M_PI * 2.0) < fabs(diff)) {
		diff += M_PI * 2.0;
	}

	if (fabs(diff - M_PI * 2.0) < fabs(diff)) {
		diff -= M_PI * 2.0;
	}

	double absDiff = fabs(diff);

	if (absDiff < step) {
		_angleH = _tAngleH;
	} else {
		_angleH += diff / absDiff * step;
	}
}

void Player::ProcessIdle(double surfaceHeight)
{
	double armStep = 0.05;

	Math::Vec<3> rArmT = {0, -0.58, 0.07};
	Math::Vec<3> lArmT = {0, 0.58, 0.07};

	Math::Vec<3> diff = rArmT - _rArm;

	if (diff.Length() <= armStep) {
		_rArm = rArmT;
	} else {
		_rArm += diff.Normalize() * armStep;
	}

	diff = lArmT - _lArm;

	if (diff.Length() <= armStep) {
		_lArm = lArmT;
	} else {
		_lArm += diff.Normalize() * armStep;
	}

	Math::Vec<3> rLegT = {0, -0.2, -1.37};
	Math::Vec<3> lLegT = {0, 0.2, -1.37};

	double legStep = 0.05;

	diff = rLegT - _rLeg;

	if (diff.Length() <= legStep) {
		_rLeg = rLegT;
	} else {
		_rLeg += diff.Normalize() * legStep;
	}

	diff = lLegT - _lLeg;

	if (diff.Length() <= legStep) {
		_lLeg = lLegT;
	} else {
		_lLeg += diff.Normalize() * legStep;
	}

	_z = 1.37 + surfaceHeight;

	_speed[0] = 0;
	_speed[1] = 0;

	if (_roll > 0) {
		_roll -= 0.05;
	} else {
		_roll = 0;
	}
}

void Player::ProcessWalk(double surfaceHeight)
{
	if (_roll > 0) {
		_roll -= 0.05;
	} else {
		_roll = 0;
	}

	Math::Vec<2> speed({_go, _strafe});
	speed *= 2.0;

	double tmp = speed[0] * cos(_camAngleH) - speed[1] * sin(_camAngleH);
	speed[1] = speed[0] * sin(_camAngleH) + speed[1] * cos(_camAngleH);
	speed[0] = tmp;

	_tAngleH = acos(speed.Normalize()[0]);

	if (speed[1] < 0) {
		_tAngleH = M_PI * 2.0 - _tAngleH;
	}

	double armStep = 0.05;

	Math::Vec<3> rArmT = {
		sin(sin(_legTime + M_PI) * M_PI / 8.0) * 1.41,
		-0.58,
		1.48 - cos(sin(_legTime + M_PI) * M_PI / 8.0) * 1.41
	};

	Math::Vec<3> lArmT = {
		sin(sin(_legTime) * M_PI / 8.0) * 1.41,
		0.58,
		1.48 - cos(sin(_legTime) * M_PI / 8.0) * 1.41
	};

	Math::Vec<3> diff = rArmT - _rArm;

	if (diff.Length() <= armStep) {
		_rArm = rArmT;
	} else {
		_rArm += diff.Normalize() * armStep;
	}

	diff = lArmT - _lArm;

	if (diff.Length() <= armStep) {
		_lArm = lArmT;
	} else {
		_lArm += diff.Normalize() * armStep;
	}

	Math::Vec<3> rLegT = {
		sin(_legTime) * 0.27,
		-0.2,
		std::max(-1.37 + cos(_legTime) * 0.4, -1.37)
	};

	Math::Vec<3> lLegT = {
		sin(_legTime + M_PI) * 0.27,
		0.2,
		std::max(-1.37 + cos(_legTime + M_PI) * 0.4, -1.37)
	};

	double legStep = 0.1;

	diff = rLegT - _rLeg;

	if (diff.Length() <= legStep) {
		_rLeg = rLegT;
	} else {
		_rLeg += diff.Normalize() * legStep;
	}

	diff = lLegT - _lLeg;

	if (diff.Length() <= legStep) {
		_lLeg = lLegT;
	} else {
		_lLeg += diff.Normalize() * legStep;
	}

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

	_speed[0] = speed[0];
	_speed[1] = speed[1];
	_z = 1.37 + surfaceHeight;
}

void Player::ProcessRun(double surfaceHeight)
{
	if (_roll < 0.5) {
		_roll += 0.05;
	} else {
		_roll = 0.5;
	}

	Math::Vec<2> speed({_go, _strafe});
	speed *= 8.0;

	double tmp = speed[0] * cos(_camAngleH) - speed[1] * sin(_camAngleH);
	speed[1] = speed[0] * sin(_camAngleH) + speed[1] * cos(_camAngleH);
	speed[0] = tmp;

	_tAngleH = acos(speed.Normalize()[0]);

	if (speed[1] < 0) {
		_tAngleH = M_PI * 2.0 - _tAngleH;
	}

	double armStep = 0.5;

	Math::Vec<3> rArmT = {
		sin(_legTime + M_PI) * 0.5 + 0.6,
		-0.58,
		0.9 + sin(_legTime + M_PI) * 0.5
	};

	Math::Vec<3> lArmT = {
		sin(_legTime) * 0.5 + 0.6,
		0.58,
		0.9 + sin(_legTime) * 0.5
	};

	Math::Vec<3> diff = rArmT - _rArm;

	if (diff.Length() <= armStep) {
		_rArm = rArmT;
	} else {
		_rArm += diff.Normalize() * armStep;
	}

	diff = lArmT - _lArm;

	if (diff.Length() <= armStep) {
		_lArm = lArmT;
	} else {
		_lArm += diff.Normalize() * armStep;
	}

	Math::Vec<3> rLegT = {
		sin(_legTime) * 0.8 + 0.5,
		-0.2,
		-1.1 + cos(_legTime) * 0.4
	};

	Math::Vec<3> lLegT = {
		sin(_legTime + M_PI) * 0.8 + 0.5,
		0.2,
		-1.1 + cos(_legTime + M_PI) * 0.4
	};

	rLegT[2] = std::max(rLegT[2], -1.1 + rLegT[0] * 0.5);
	lLegT[2] = std::max(lLegT[2], -1.1 + lLegT[0] * 0.5);

	double legStep = 0.5;

	diff = rLegT - _rLeg;

	if (diff.Length() <= legStep) {
		_rLeg = rLegT;
	} else {
		_rLeg += diff.Normalize() * legStep;
	}

	diff = lLegT - _lLeg;

	if (diff.Length() <= legStep) {
		_lLeg = lLegT;
	} else {
		_lLeg += diff.Normalize() * legStep;
	}

	if (speed.Length() > 0) {
		_legTime += _legStep * 2.0;
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

	_speed[0] = speed[0];
	_speed[1] = speed[1];
	_z = 1.0 + surfaceHeight;
}

void Player::ProcessDash(double surfaceHeight)
{
	Math::Vec<2> speed({_go, _strafe});

	if (speed.Length() == 0) {
		speed[0] = 1;
	}

	double tmp = speed[0] * cos(_camAngleH) - speed[1] * sin(_camAngleH);
	speed[1] = speed[0] * sin(_camAngleH) + speed[1] * cos(_camAngleH);
	speed[0] = tmp;

	if (_inDash) {
		speed = _dashDir;
	}

	_tAngleH = acos(speed.Normalize()[0]);

	if (speed[1] < 0) {
		_tAngleH = M_PI * 2.0 - _tAngleH;
	}

	double armStep = 0.5;

	Math::Vec<3> rArmT = {0.6, -0.58, 0.9};
	Math::Vec<3> lArmT = {0.6, 0.58, 0.9};

	Math::Vec<3> diff = rArmT - _rArm;

	if (diff.Length() <= armStep) {
		_rArm = rArmT;
	} else {
		_rArm += diff.Normalize() * armStep;
	}

	diff = lArmT - _lArm;

	if (diff.Length() <= armStep) {
		_lArm = lArmT;
	} else {
		_lArm += diff.Normalize() * armStep;
	}

	Math::Vec<3> rLegT = {
		0.2,
		-0.2,
		-0.4
	};

	Math::Vec<3> lLegT = {
		0.2,
		0.2,
		-0.4
	};

	double legStep = 0.05;

	diff = rLegT - _rLeg;

	if (diff.Length() <= legStep) {
		_rLeg = rLegT;
	} else {
		_rLeg += diff.Normalize() * legStep;
	}

	diff = lLegT - _lLeg;

	if (diff.Length() <= legStep) {
		_lLeg = lLegT;
	} else {
		_lLeg += diff.Normalize() * legStep;
	}

	if (!_inDash) {
		_inDash = true;
		--_dash;
		_dashDir = speed.Normalize();
	} else {
		if (_roll < M_PI * 2.0) {
			_roll += 0.3;
		} else {
			_roll = 0;
			_inDash = false;
		}
	}

	_speed[0] = _dashDir[0] * 20.0;
	_speed[1] = _dashDir[1] * 20.0;
	_z = 1.4 + surfaceHeight + sin(_roll / 2.0);
}

void Player::ProcessBlock()
{
	double blockStep = 0.1;

	if (_block) {
		Math::Vec<3> diff = Math::Vec<3>({-1, -1, 0}) - _rArmDir;

		if (diff.Length() > blockStep) {
			_rArmDir += diff.Normalize() * blockStep;
		} else {
			_rArmDir = {-1, -1, 0};
		}

		_lArmDir = {-1, 0, 0};
	} else {
		Math::Vec<3> diff = Math::Vec<3>({-1, 0, 0}) - _rArmDir;

		if (diff.Length() > blockStep) {
			_rArmDir += diff.Normalize() * blockStep;
		} else {
			_rArmDir = {-1, 0, 0};
		}

		_lArmDir = {-1, 0, 0};
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
	} else if (key == GLFW_KEY_LEFT_SHIFT) {
		if (action == GLFW_PRESS) {
			_sprint = true;
		} else if (action == GLFW_RELEASE) {
			_sprint = false;
		}
	}
}

bool Player::MouseMoveRaw(double xoffset, double yoffset)
{
	if (_gameGlobal->Paused) {
		return false;
	}

	_camAngleH -= xoffset / 100.0;
	_camAngleV += yoffset / 100.0;

	if (_camAngleH < 0) {
		_camAngleH += M_PI * 2.0;
	} else if (_camAngleH >= M_PI * 2.0) {
		_camAngleH -= M_PI * 2.0;
	}

	_camAngleV = std::clamp(_camAngleV, -M_PI / 2.01, M_PI / 2.01);

	return true;
}

bool Player::Scroll(double xoffset, double yoffset)
{
	if (_gameGlobal->Paused) {
		return false;
	}

	_cameraDist += yoffset * 0.1;

	if (_cameraDist < 0.5) {
		_cameraDist = 0.5;
	}

	return true;
}

bool Player::MouseButton(int button, int action, int mods)
{
	if (_gameGlobal->Paused) {
		return false;
	}

	if (button == GLFW_MOUSE_BUTTON_4) {
		if (action == GLFW_PRESS) {
			++_dash;
		}
	} else if (button == GLFW_MOUSE_BUTTON_2) {
		if (action == GLFW_PRESS) {
			_block = true;
		} else if (action == GLFW_RELEASE) {
			_block = false;
		}
	}

	return true;
}
