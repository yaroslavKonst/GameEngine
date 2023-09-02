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
	_pos = glm::vec3(0.0, 0.0, 5.0);
	_angleH = 0;
	_angleV = 0;
	_go = 0;
	_strafe = 0;
	_jump = false;
	_vspeed = 0;
	_lightActive = false;
	_ship = ship;
	_buildMode = false;
	_buildCamCoeff = 0;
	_actionRequested = false;

	std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(0.1, -0.1, 0.0));
	vertices.push_back(glm::vec3(0.1, 0.1, 0.0));
	vertices.push_back(glm::vec3(-0.1, 0.0, 0.0));
	vertices.push_back(glm::vec3(0.1, -0.1, 1.0));
	vertices.push_back(glm::vec3(0.1, 0.1, 1.0));
	vertices.push_back(glm::vec3(-0.1, 0.0, 1.0));
	vertices.push_back(glm::vec3(0.1, -0.1, 1.5));
	vertices.push_back(glm::vec3(0.1, 0.1, 1.5));
	vertices.push_back(glm::vec3(-0.1, 0.0, 1.5));
	vertices.push_back(glm::vec3(0.1, -0.1, 1.9));
	vertices.push_back(glm::vec3(0.1, 0.1, 1.9));
	vertices.push_back(glm::vec3(-0.1, 0.0, 1.9));
	vertices.push_back(glm::vec3(0.1, -0.1, 0.5));
	vertices.push_back(glm::vec3(0.1, 0.1, 0.5));
	vertices.push_back(glm::vec3(-0.1, 0.0, 0.5));

	std::vector<uint32_t> indices = {0, 1, 2};

	SetObjectVertices(vertices);
	SetObjectIndices(indices);
	SetObjectCenter({0.0f, 0.0f, 1.5f});
	SetObjectMatrix(glm::mat4(1.0));
	SetObjectDynamic(true);

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
	_cornerTextBox->SetPosition(-0.8, -0.8);
	_cornerTextBox->SetTextSize(0.15);
	_cornerTextBox->SetText("Build mode");
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
				_buildMode = true;
				_buildPos = {0, 0, 0};
				_buildCamPos = {0, 0, 0};
				_buildRotation = {0, 0, 0};
				Logger::Verbose() << "Build mode on.";
			}
		}
	}

	if (!_buildMode) {
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
				_actionRequested = true;
			}
			_mutex.unlock();
		}
	}
}

bool Player::MouseMoveRaw(
	double xoffset,
	double yoffset)
{
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

	_vspeed -= 0.098 / 1;

	_pos.x += hspeed.x / 10;
	_pos.y += hspeed.y / 10;
	_pos.z += _vspeed / 100;

	/*SetObjectSpeed(glm::vec3(
		hspeed.x / 10,
		hspeed.y / 10,
		_vspeed / 100));*/

	SetObjectMatrix(
		glm::rotate(glm::translate(glm::mat4(1.0), _pos),
			glm::radians(_angleH), glm::vec3(0, 0, 1)));
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

	if (effect.z > 0 && _vspeed < 0) {
		_pos.z += effect.z - 0.0000001;

		if (fabs(_vspeed) > 0.0001) {
			_vspeed = -_vspeed * 0.4;
		} else {
			if (effect.z < 0.0000001) {
				_vspeed = 0;
			} else {
				_vspeed = 0.1;
			}
		}
	}

	if (effect.z < 0 && _vspeed > 0) {
		_vspeed = 0;
		_pos.z += effect.z;
	}

	if (effect.z > 0 && _jump) {
		_vspeed = 6;
	}

	if (fabs(effect.x) + fabs(effect.y) > 0.0001) {
		_pos.x += effect.x;
		_pos.y += effect.y;
	}

	glm::vec3 cameraPosition = _pos + glm::vec3(0, 0, 1.85);
	glm::vec3 cameraDirection = glm::vec3(
		hdir * cosf(glm::radians(_angleV)),
		sinf(glm::radians(_angleV)));

	//cameraPosition -= glm::normalize(cameraDirection) * 1.5f;

	glm::vec3 buildCamPosTarget = glm::vec3(_buildPos);

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
		nullptr);

	if (object.Code) {
		_centerTextBox->SetText("[E] Object action");
		_centerTextBox->Activate();

		if (_actionRequested) {
			FloorCommBlock* block = static_cast<FloorCommBlock*>(
				object.object);

			block->SetPowerCable(!block->GetPowerCable());
		}
	} else {
		_centerTextBox->Deactivate();
	}

	if (_buildMode) {
		_cornerTextBox->Activate();
	} else {
		_cornerTextBox->Deactivate();
	}

	_actionRequested = false;

	_mutex.unlock();
}
