#include "player.h"

#include "../Engine/Assets/ScriptHandler.h"
#include "../Engine/Math/transform.h"

#include "../Engine/Logger/logger.h"

Player::Player(
	Common common,
	Shuttle* ship,
	GravityField* gf,
	Planet* planet)
{
	_common = common;

	_planet = planet;

	_gf = gf;
	_pos = {0.0, -3.0, 0.1};
	_dirUp = {0, 0, 1};
	_dirF = {0, 1, 0};
	_dirR = {1, 0, 0};
	_matrix = Math::Mat<4>(1.0);
	_angleH = 0;
	_angleV = 0;
	_go = 0;
	_strafe = 0;
	_jump = false;
	_lightActive = false;
	_ship = ship;
	_buildMode = false;
	_flightMode = false;
	_buildCamCoeff = 0;
	_actionERequested = false;
	_actionRRequested = false;
	_actionFRequested = false;

	_sword = new Sword(_common, nullptr);

	SoftPhysicsValues::Vertex phVertex;
	phVertex.Mass = 80;
	phVertex.Mu = 0.3;
	phVertex.Bounciness = 0.2;
	phVertex.Position = {0, 0, 1};

	SoftPhysicsParams.Vertices = {phVertex};

	SetInputEnabled(true);

	_light.Type = Light::Type::Spot;
	_light.Color = {1.7, 1.7, 1.7};
	_light.Angle = 30;
	_light.AngleFade = 10;

	_common.video->RegisterLight(&_light);
	_common.video->Subscribe(this);

	_centerTextBox = new TextBox(_common.video, _common.textHandler);
	_centerTextBox->SetPosition(0.03, 0.03);
	_centerTextBox->SetTextSize(0.1);
	_centerTextBox->SetText("Center text");
	_centerTextBox->SetTextColor({1, 1, 1, 1});
	_centerTextBox->SetDepth(0);
	_centerTextBox->Activate();

	_cornerTextBox = new TextBox(_common.video, _common.textHandler);
	_cornerTextBox->SetPosition(-0.9, -0.9);
	_cornerTextBox->SetTextSize(0.05);
	_cornerTextBox->SetText(
		"Build mode.\n\n[WASD] Move\n[E] Set\n[Q] Remove\n\
[T] Change type\n[R] Next layer\n[F] Previous layer\n[Scroll] Rotate block");
	_cornerTextBox->SetTextColor({1, 1, 1, 1});
	_cornerTextBox->SetDepth(0);
	_cornerTextBox->Activate();

	auto td = Loader::LoadImage("Images/Cross.png");
	_crossTexture = _common.video->LoadTexture(td);

	_cross.RectangleParams.Position = {-0.02, -0.02, 0.02, 0.02};
	_cross.RectangleParams.TexCoords = {0, 0, 1, 1};
	_cross.RectangleParams.Depth = 0;
	_cross.RectangleParams.Texture = _crossTexture;
	_cross.DrawParams.Enabled = true;

	_common.video->RegisterRectangle(&_cross);
}

Player::~Player()
{
	_common.video->RemoveRectangle(&_cross);

	_centerTextBox->Deactivate();
	delete _centerTextBox;
	_cornerTextBox->Deactivate();
	delete _cornerTextBox;

	delete _sword;

	_common.video->Unsubscribe(this);
	_common.video->RemoveLight(&_light);
	_common.video->UnloadTexture(_crossTexture);
}

void Player::Key(
	int key,
	int scancode,
	int action,
	int mods)
{
	if (_flightMode) {
		if (key == GLFW_KEY_F) {
			if (action == GLFW_PRESS) {
				_flightMode = false;
			}
		}

		return;
	}

	if (key == GLFW_KEY_C) {
		if (action == GLFW_PRESS) {
			_common.video->ToggleRawMouseInput();
		}
	} else if (key == GLFW_KEY_SPACE) {
		if (action == GLFW_PRESS) {
			_jump = true;
		} else if (action == GLFW_RELEASE) {
			_jump = false;
		}
	} else if (key == GLFW_KEY_Z) {
		if (action == GLFW_PRESS) {
			if (_lightActive) {
				_light.Enabled = false;
				_lightActive = false;
			} else {
				_light.Enabled = true;
				_lightActive = true;
			}
		}
	}

	if (!(_buildMode || _flightMode)) {
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
				_strafe += 1;
			} else if (action == GLFW_RELEASE) {
				_strafe -= 1;
			}
		} else if (key == GLFW_KEY_A) {
			if (action == GLFW_PRESS) {
				_strafe -= 1;
			} else if (action == GLFW_RELEASE) {
				_strafe += 1;
			}
		} else if (key == GLFW_KEY_E) {
			if (action == GLFW_PRESS) {
				_actionERequested = true;
			}
		} else if (key == GLFW_KEY_R) {
			if (action == GLFW_PRESS) {
				_actionRRequested = true;
			}
		} else if (key == GLFW_KEY_F) {
			if (action == GLFW_PRESS) {
				_actionFRequested = true;
			}
		}
	}
}

bool Player::MouseMoveRaw(
	double xoffset,
	double yoffset)
{
	if (_buildMode || _flightMode) {
		return false;
	}

	_angleH -= xoffset * 0.1;
	_angleV -= yoffset * 0.1;

	_angleV = std::clamp(_angleV, -85.0, 85.0);

	return true;
}

bool Player::MouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			_sword->Use();
		}
	}

	return true;
}

void Player::TickEarly()
{
	Math::Vec<3> gravityF = (*_gf)(_pos);
	gravityF *= 80.0f;

	SoftPhysicsParams.Force = gravityF;

	Math::Vec<3> tDirUp = -gravityF.Normalize();

	double angleCos = tDirUp.Dot(_dirUp.Normalize());

	if (abs(angleCos) < 0.999999) {
		Math::Vec<3> rotAxis = tDirUp.Cross(_dirUp);

		double angle = acos(angleCos);
		double angleLim = M_PI / 1000.0;

		angleLim = angle / 100.0 + M_PI / 10000.0;
		angleLim = std::clamp<float>(
			angleLim,
			M_PI / 10000.0,
			M_PI / 1000.0);

		if (angle > angleLim) {
			angle = angleLim;
		}

		Math::Mat<4> rotMat = Math::Rotate(
			angle,
			-rotAxis.Normalize());

		_matrix = rotMat * _matrix;

		_dirUp = rotMat * Math::Vec<4>(_dirUp, 0.0);
		_dirF = rotMat * Math::Vec<4>(_dirF, 0.0);
		_dirR = rotMat * Math::Vec<4>(_dirR, 0.0);
	}

	if (_angleH != 0) {
		Math::Mat<4> rotMat = Math::Rotate(
			_angleH,
			_dirUp.Normalize(),
			Math::Degrees);

		_matrix = rotMat * _matrix;

		_dirUp = rotMat * Math::Vec<4>(_dirUp, 0.0);
		_dirF = rotMat * Math::Vec<4>(_dirF, 0.0);
		_dirR = rotMat * Math::Vec<4>(_dirR, 0.0);

		_angleH = 0;
	}

	SoftPhysicsParams.Vertices[0].Position = _pos;
}

void Player::Tick()
{
	_pos = SoftPhysicsParams.Vertices[0].Position;

	_cornerTextBox->SetText(
		"Player: \nSpeed: " +
		std::to_string(
			SoftPhysicsParams.Vertices[0].Speed.Length()));
	_cornerTextBox->Activate();

	Math::Mat<4> translateMat = Math::Translate(
		_pos + _dirF + _dirUp + _dirR * 0.5);

	Math::Mat<4> swordMat(1.0);

	swordMat[0][0] = -_dirR[0];
	swordMat[0][1] = -_dirR[1];
	swordMat[0][2] = -_dirR[2];

	swordMat[1][0] = _dirF[0];
	swordMat[1][1] = _dirF[1];
	swordMat[1][2] = _dirF[2];

	swordMat[2][0] = _dirUp[0];
	swordMat[2][1] = _dirUp[1];
	swordMat[2][2] = _dirUp[2];

	swordMat[0][1] *= -1;
	swordMat[1][1] *= -1;
	swordMat[2][1] *= -1;

	_sword->ModelParams.Matrix = translateMat * swordMat;
	_sword->Update(0.01);

	_planet->Update(_pos);

	Math::Vec<3> hspeed = _dirF * (double)_go +
		_dirR * (double)_strafe;

	SoftPhysicsParams.Vertices[0].Force = hspeed * 600.0;

	if (_jump) {
		SoftPhysicsParams.Vertices[0].Force += _dirUp * 1000.0;
	}

	if (_flightMode) {
		Math::Mat<4> matrix = _ship->GetMatrix();

		Math::Vec<3> offset = matrix * Math::Vec<4>({0, -1, -1, 0});

		offset = offset.Normalize() * 0.5;

		_pos = Math::Vec<3>(matrix * Math::Vec<4>({0, 0, 5, 1.0})) +
			offset;

		SoftPhysicsParams.Vertices[0].Position = _pos;
		SoftPhysicsParams.Vertices[0].Speed = 0.0;
	}

	if (!_flightMode) {
		Math::Mat<4> cameraDirectionRotMat = Math::Rotate(
			_angleV,
			_dirR,
			Math::Degrees);

		Math::Vec<3> cameraPosition = _pos + _dirUp * 1.85;
		Math::Vec<3> cameraDirection =
			cameraDirectionRotMat * Math::Vec<4>(_dirF, 0.0);

		_common.video->SetCameraPosition(cameraPosition);
		_common.video->SetCameraDirection(cameraDirection);
		_common.video->SetCameraUp(_dirUp);
	}

	_light.Position = _pos + _dirUp * 1.2 + _dirR * -0.3;
	_light.Direction = _dirF;

	PhysicalEngine::RayCastResult object =
		_common.physicalEngine->RayCast(
			_pos + _dirUp * 1.85,
			_dirF,
			3,
			nullptr,
			{});

	if (object.Code == 1) {
		_centerTextBox->SetText(
			"Communications\n[E] Power\n[R] Data");
		_centerTextBox->Activate();
		_cross.DrawParams.ColorMultiplier = {0.3, 1.0, 0.3, 1.0};
	} else if (object.Code == 2 && !_flightMode) {
		_centerTextBox->SetText(
			"FlightControl\n[E] Use");
		_centerTextBox->Activate();
		_cross.DrawParams.ColorMultiplier = {0.3, 1.0, 0.3, 1.0};
	} else {
		_centerTextBox->Deactivate();
		_cross.DrawParams.ColorMultiplier = {1.0, 1.0, 1.0, 1.0};
	}

	if (_actionFRequested) {
		_ship->ActivateFlight();
		_flightMode = true;
		_ship->SetInputEnabled(true);
	}

	_actionERequested = false;
	_actionRRequested = false;
	_actionFRequested = false;
}

Sword::Sword(Common common, Math::Mat<4>* extMat)
{
	_common = common;

	ModelParams.Model = _common.video->LoadModel(Loader::LoadModel(
		"Models/Player/Sword.obj"));
	TextureParams.SetAll(_common.video->LoadTexture(Loader::LoadImage(
		"Models/Player/Sword.png")));

	ModelParams.ExternalMatrix = extMat;

	DrawParams.Enabled = true;
	ModelParams.Matrix = Math::Mat<4>(1.0);
	ModelParams.Matrix[1][3] = 2.0;

	_common.video->RegisterModel(this);

	_animation = ScriptHandler::LoadAnimation("Models/Player/Sword.txt");
	_time = 0;
}

Sword::~Sword()
{
	_common.video->RemoveModel(this);

	_common.video->UnloadModel(ModelParams.Model);
	_common.video->UnloadTexture(TextureParams.Diffuse);
}

void Sword::Update(float time)
{
	if (_time == 0) {
		return;
	}

	_time += time;

	if (_time >= 6) {
		_time = 0;
	}

	ModelParams.InnerMatrix = {
		_animation.GetTransform(_time, Math::Mat<4>(1.0))};
}

void Sword::Use()
{
	_animation = ScriptHandler::LoadAnimation("../Sword.txt");

	if (_time == 0) {
		_time += 0.0001;
	}
}
