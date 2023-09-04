#include "player.h"

#include "../Logger/logger.h"

Player::Player(
	Video* video,
	CollisionEngine* rayEngine,
	Ship* ship,
	TextHandler* textHandler)
{
	_video = video;
	_rayEngine = rayEngine;
	_textHandler = textHandler;
	_pos = glm::vec3(0.0, 0.0, 2.0);
	_angleH = 0;
	_angleV = 0;
	_go = 0;
	_strafe = 0;
	_jump = false;
	_vspeed = 0;
	_lightActive = false;
	_ship = ship;
	_buildMode = false;
	_flightMode = false;
	_buildCamCoeff = 0;
	_actionERequested = false;
	_actionRRequested = false;
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

	_light.SetLightType(Light::Type::Spot);
	_light.SetLightColor({0.7, 0.7, 0.7});
	_light.SetLightAngle(30);
	_light.SetLightAngleFade(10);

	_video->RegisterLight(&_light);
	_video->GetInputControl()->Subscribe(this);

	float ratio = 1.0 / _video->GetScreenRatio();

	_centerTextBox = new TextBox(_video, _textHandler);
	_centerTextBox->SetPosition(0.03 * ratio, 0.03);
	_centerTextBox->SetTextSize(0.1);
	_centerTextBox->SetText("Center text");
	_centerTextBox->SetTextColor({1, 1, 1, 1});
	_centerTextBox->SetDepth(0);
	_centerTextBox->Activate();

	_cornerTextBox = new TextBox(_video, _textHandler);
	_cornerTextBox->SetPosition(-0.9, -0.9);
	_cornerTextBox->SetTextSize(0.1);
	_cornerTextBox->SetText(
		"Build mode\n\n[WASD] Move\n[E] Set\n[Q] Remove\n\
[T] Change type\n[R] Next layer\n[F] Previous layer\n[Scroll] Rotate block");
	_cornerTextBox->SetTextColor({1, 1, 1, 1});
	_cornerTextBox->SetDepth(0);

	int tw;
	int th;
	auto td = Loader::LoadImage("Images/Cross.png", tw, th);
	_crossTexture = _video->GetTextures()->AddTexture(tw, th, td);

	_cross.SetRectanglePosition({-0.02 * ratio, -0.02, 0.02 * ratio, 0.02});
	_cross.SetRectangleTexCoords({0, 0, 1, 1});
	_cross.SetRectangleDepth(0);
	_cross.SetTexture({_crossTexture});
	_cross.SetDrawEnabled(true);

	_video->RegisterRectangle(&_cross);
}

Player::~Player()
{
	_video->RemoveRectangle(&_cross);

	_centerTextBox->Deactivate();
	delete _centerTextBox;
	delete _cornerTextBox;

	_video->GetInputControl()->UnSubscribe(this);
	_video->RemoveLight(&_light);
}

void Player::Key(
	int key,
	int scancode,
	int action,
	int mods)
{
	if (_flightMode) {
		if (key == GLFW_KEY_Q) {
			if (action == GLFW_PRESS) {
				_flightMode = false;
			}
		}

		return;
	}

	if (key == GLFW_KEY_C) {
		if (action == GLFW_PRESS) {
			_mutex.lock();
			_video->GetInputControl()->
				ToggleRawMouseInput();
			_mutex.unlock();
		}
	} else if (key == GLFW_KEY_SPACE) {
		_mutex.lock();
		if (action == GLFW_PRESS) {
			_jump = true;
		} else if (action == GLFW_RELEASE) {
			_jump = false;
		}
		_mutex.unlock();
	} else if (key == GLFW_KEY_Z) {
		if (action == GLFW_PRESS) {
			if (_lightActive) {
				_light.SetLightActive(false);
				_lightActive = false;
			} else {
				_light.SetLightActive(true);
				_lightActive = true;
			}
		}
	} else if (key == GLFW_KEY_B) {
		if (action == GLFW_PRESS) {
			if (_buildMode) {
				_buildMode = false;
				Logger::Verbose() << "Build mode off.";
			} else {
				_ship->SetInputEnabled(true);
				_ship->ActivateBuild();
				_buildMode = true;
				Logger::Verbose() << "Build mode on.";
			}
		}
	}

	if (!(_buildMode || _flightMode)) {
		if (key == GLFW_KEY_W) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_go += 1;
			} else if (action == GLFW_RELEASE) {
				_go -= 1;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_S) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_go -= 1;
			} else if (action == GLFW_RELEASE) {
				_go += 1;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_D) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_strafe += 1;
			} else if (action == GLFW_RELEASE) {
				_strafe -= 1;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_A) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_strafe -= 1;
			} else if (action == GLFW_RELEASE) {
				_strafe += 1;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_E) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_actionERequested = true;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_R) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_actionRRequested = true;
			}
			_mutex.unlock();
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

	_mutex.lock();
	_angleH += xoffset * 0.1;
	_angleV += yoffset * 0.1;

	if (_angleH < 0) {
		_angleH += 360;
	} else if (_angleH >= 360) {
		_angleH -= 360;
	}

	_angleV = std::clamp(_angleV, -85.0f, 85.0f);
	_mutex.unlock();

	return true;
}

void Player::TickEarly()
{
	glm::vec2 hdir(
		sinf(glm::radians(_angleH)),
		cosf(glm::radians(_angleH)));

	glm::vec2 hdirStrafe(
		sinf(glm::radians(_angleH + 90)),
		cosf(glm::radians(_angleH + 90)));

	glm::vec2 hspeed = hdir * (float)_go +
		hdirStrafe * (float)_strafe;

	if (!_flightMode) {
		_vspeed -= 9.8 / 100;

		_pos.x += hspeed.x / 10;
		_pos.y += hspeed.y / 10;
		_pos.z += _vspeed / 100;

		SetObjectMatrix(
			glm::rotate(glm::translate(glm::mat4(1.0), _pos),
				glm::radians(_angleH), glm::vec3(0, 0, 1)));
	}
}

void Player::Tick()
{
	_mutex.lock();

	glm::vec2 hdir(
		sinf(glm::radians(_angleH)),
		cosf(glm::radians(_angleH)));

	glm::vec2 hdirStrafe(
		sinf(glm::radians(_angleH + 90)),
		cosf(glm::radians(_angleH + 90)));

	glm::vec3 effect = GetObjectEffect();

	_pos += effect;

	if (effect.z > 0 && _vspeed < 0) {
		if (fabs(_vspeed) > 0.0001) {
			_vspeed = -_vspeed * 0.4;
		} else {
			_vspeed = 0;
		}
	}

	if (effect.z < 0 && _vspeed > 0) {
		_vspeed = 0;
	}

	if (effect.z > 0 && _jump) {
		_vspeed = 6;
	}

	if (_flightMode) {
		glm::mat4 matrix =
			*_activeFlightControl->GetObjectExternalMatrix() *
			_activeFlightControl->GetObjectMatrix();

		glm::vec3 offset = matrix * glm::vec4(0, -1, -1, 0);

		offset = glm::normalize(offset) * 0.5f;

		_pos = glm::vec3(matrix * glm::vec4(
			_activeFlightControl->GetObjectCenter(), 1.0f)) +
			offset;

		SetObjectMatrix(
			glm::rotate(glm::translate(glm::mat4(1.0), _pos),
				glm::radians(_angleH), glm::vec3(0, 0, 1)));

		glm::vec3 cameraPosition = _pos + glm::vec3(0, 0, 1.85);
		_video->SetCameraPosition(cameraPosition);
	}

	glm::vec3 cameraPosition = _pos + glm::vec3(0, 0, 1.85);
	glm::vec3 cameraDirection = glm::vec3(
		hdir * cosf(glm::radians(_angleV)),
		sinf(glm::radians(_angleV)));

	if (_flightMode) {
		cameraPosition -= glm::normalize(cameraDirection) * 20.0f;
	}

	glm::vec3 buildCamPosTarget = glm::vec3(0.0);

	float buildCamDist = glm::length(_buildCamPos - buildCamPosTarget);

	if (buildCamDist > 0.04) {
		_buildCamPos -=
			glm::normalize(_buildCamPos - buildCamPosTarget) *
				std::clamp(0.008f * buildCamDist, 0.04f, 1.0f);
	} else {
		_buildCamPos = buildCamPosTarget;
	}

	cameraPosition = cameraPosition * (1.0f - _buildCamCoeff) +
		(_buildCamPos + glm::vec3(-5, -2, 10)) *
		_buildCamCoeff;

	cameraDirection = cameraDirection * (1.0f - _buildCamCoeff) +
		(_buildCamPos - cameraPosition) *
		_buildCamCoeff;

	if (!_buildMode) {
		_buildCamCoeff -= 0.02;
	} else {
		_buildCamCoeff += 0.02;
	}

	_buildCamCoeff = std::clamp(_buildCamCoeff, 0.0f, 1.0f);

	_video->SetCameraPosition(cameraPosition);
	_video->SetCameraDirection(cameraDirection);

	_light.SetLightPosition(_pos + glm::vec3(0, 0, 1.2) +
			glm::vec3(-hdirStrafe * 0.3f, 0.0f));
	_light.SetLightDirection(glm::vec3(
			hdir * cosf(glm::radians(_angleV)),
			sinf(glm::radians(_angleV))));

	CollisionEngine::RayCastResult object = _rayEngine->RayCast(
		_pos + glm::vec3(0, 0, 1.85),
		glm::vec3(
			hdir * cosf(glm::radians(_angleV)),
			sinf(glm::radians(_angleV))),
		3,
		nullptr,
		{this});

	if (object.Code == 1) {
		_centerTextBox->SetText(
			"Communications\n[E] Power\n[R] Data");
		_centerTextBox->Activate();
		_cross.SetColorMultiplier({0.3, 1.0, 0.3, 1.0});

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
		_cross.SetColorMultiplier({0.3, 1.0, 0.3, 1.0});

		if (_actionERequested) {
			FlightControl* block = static_cast<FlightControl*>(
				object.object);

			_activeFlightControl = block;

			_ship->ActivateFlight(block);
			_flightMode = true;
			_ship->SetInputEnabled(true);
		}
	} else {
		_centerTextBox->Deactivate();
		_cross.SetColorMultiplier({1.0, 1.0, 1.0, 1.0});
	}

	if (_buildMode) {
		_cornerTextBox->Activate();
	} else {
		_cornerTextBox->Deactivate();
	}

	_actionERequested = false;
	_actionRRequested = false;

	_mutex.unlock();
}
