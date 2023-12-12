#include "player.h"

#include "../Logger/logger.h"

Player::Player(
	Common common,
	Shuttle* ship,
	GravityField* gf,
	Planet* planet)
{
	_common = common;

	_planet = planet;

	_gf = gf;
	_pos = glm::vec3(0.0, -3.0, 0.1);
	_dirUp = {0, 0, 1};
	_dirF = {0, 1, 0};
	_dirR = {1, 0, 0};
	_matrix = glm::mat4(1.0);
	_angleH = 0;
	_angleV = 0;
	_go = 0;
	_strafe = 0;
	_jump = false;
	_speed = {0, 0, 0};
	_lightActive = false;
	_ship = ship;
	_buildMode = false;
	_flightMode = false;
	_buildCamCoeff = 0;
	_actionERequested = false;
	_actionRRequested = false;
	_actionFRequested = false;
	_activeFlightControl = nullptr;

	auto collision = Loader::LoadModel("Models/Player/Collision.obj");
	SetObjectVertices(collision.Vertices);
	SetObjectIndices(collision.Indices);
	SetObjectNormals(collision.Normals);
	SetObjectCenter();
	SetObjectMatrix(glm::mat4(1.0));
	SetObjectDynamic(true);
	SetObjectSphereCenter({0, 0, 0.45});
	SetObjectSphereRadius(0.45);

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

	auto td = Loader::LoadImage("Images/Cross.png");
	_crossTexture = _common.video->AddTexture(td);

	_cross.RectangleParams.Position = {-0.02, -0.02, 0.02, 0.02};
	_cross.RectangleParams.TexCoords = {0, 0, 1, 1};
	_cross.RectangleParams.Depth = 0;
	_cross.TextureParams.SetAll(_crossTexture);
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

	_common.video->Unsubscribe(this);
	_common.video->RemoveLight(&_light);
	_common.video->RemoveTexture(_crossTexture);
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
				SetObjectDomain(0);
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

	_angleV = std::clamp(_angleV, -85.0f, 85.0f);

	return true;
}

void Player::TickEarly()
{
	glm::vec3 gravityF = (*_gf)(_pos);
	gravityF *= 80.0f;

	glm::vec3 tDirUp = -glm::normalize(gravityF);

	float angleCos = glm::dot(
		glm::normalize(tDirUp),
		glm::normalize(_dirUp));

	if (fabs(angleCos) < 0.999999) {
		glm::vec3 rotAxis = glm::cross(tDirUp, _dirUp);

		float angle = acos(angleCos);
		float angleLim = M_PI / 1000.0;

		angleLim = angle / 100.0 + M_PI / 10000.0;
		angleLim = std::clamp<float>(
			angleLim,
			M_PI / 10000.0,
			M_PI / 1000.0);

		if (angle > angleLim) {
			angle = angleLim;
		}

		glm::mat4 rotMat = glm::rotate(
			glm::mat4(1.0),
			angle,
			-glm::normalize(rotAxis));

		_matrix = rotMat * _matrix;

		_dirUp = rotMat * glm::vec4(_dirUp, 0.0f);
		_dirF = rotMat * glm::vec4(_dirF, 0.0f);
		_dirR = rotMat * glm::vec4(_dirR, 0.0f);
	}

	if (_angleH != 0) {
		glm::mat4 rotMat = glm::rotate(
			glm::mat4(1.0),
			glm::radians(_angleH),
			glm::normalize(_dirUp));

		_matrix = rotMat * _matrix;

		_dirUp = rotMat * glm::vec4(_dirUp, 0.0f);
		_dirF = rotMat * glm::vec4(_dirF, 0.0f);
		_dirR = rotMat * glm::vec4(_dirR, 0.0f);

		_angleH = 0;
	}

	if (!_flightMode) {
		_speed += gravityF / 100.0f / 80.0f;

		_pos += _speed / 100.0f;

		SetObjectMatrix(_matrix * glm::translate(glm::mat4(1.0), _pos));
	}
}

void Player::Tick()
{
	glm::vec3 effect = GetObjectEffect();

	_pos += effect;

	_planet->Update(_pos);

	if (glm::dot(effect, _dirUp) > 0) {
		glm::vec3 hspeed = _dirF * (float)_go +
			_dirR * (float)_strafe;

		_speed = hspeed * 6.0f;

		if (_jump) {
			_speed += _dirUp * 6.0f;
		}
	}

	if (_flightMode) {
		glm::mat4 matrix = _ship->GetMatrix();

		glm::vec3 offset = matrix * glm::vec4(0, -1, -1, 0);

		offset = glm::normalize(offset) * 0.5f;

		_pos = glm::vec3(matrix * glm::vec4(
			0, 0, 5, 1.0f)) +
			offset;

		SetObjectMatrix(_matrix * glm::translate(glm::mat4(1.0), _pos));
	}

	if (!_flightMode) {
		glm::mat4 cameraDirectionRotMat = glm::rotate(
			glm::mat4(1.0),
			glm::radians(_angleV),
			_dirR);

		glm::vec3 cameraPosition = _pos + _dirUp * 1.85f;
		glm::vec3 cameraDirection =
			cameraDirectionRotMat * glm::vec4(_dirF, 0.0f);

		_common.video->SetCameraPosition(cameraPosition);
		_common.video->SetCameraDirection(cameraDirection);
		_common.video->SetCameraUp(_dirUp);
	}

	_light.Position = _pos + _dirUp * 1.2f + _dirR * -0.3f;
	_light.Direction = _dirF;

	CollisionEngine::RayCastResult object =
		_common.collisionEngine->RayCast(
			_pos + _dirUp * 1.85f,
			_dirF,
			3,
			nullptr,
			{this});

	if (object.Code == 1) {
		_centerTextBox->SetText(
			"Communications\n[E] Power\n[R] Data");
		_centerTextBox->Activate();
		_cross.DrawParams.ColorMultiplier = {0.3, 1.0, 0.3, 1.0};

		if (_actionERequested) {
			FloorCommBlock* block = static_cast<FloorCommBlock*>(
				object.object);

			block->SetPowerCable(!block->GetPowerCable());
		}

		if (_actionRRequested) {
			FloorCommBlock* block = static_cast<FloorCommBlock*>(
				object.object);

			block->SetDataCable(!block->GetDataCable());
		}
	} else if (object.Code == 2 && !_flightMode) {
		_centerTextBox->SetText(
			"FlightControl\n[E] Use");
		_centerTextBox->Activate();
		_cross.DrawParams.ColorMultiplier = {0.3, 1.0, 0.3, 1.0};

		if (_actionERequested) {
			FlightControl* block = static_cast<FlightControl*>(
				object.object);

			_activeFlightControl = block;

			_ship->ActivateFlight();
			_flightMode = true;
			_ship->SetInputEnabled(true);
			SetObjectDomain(1);
		}
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
