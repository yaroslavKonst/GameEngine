#include "player.h"

#include "../Logger/logger.h"

Player::Player(
	Video* video,
	CollisionEngine* rayEngine,
	Ship* ship)
{
	_video = video;
	_rayEngine = rayEngine;
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

	// Pyramid
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
	vertices.push_back(glm::vec3(0.1, -0.1, 2.0));
	vertices.push_back(glm::vec3(0.1, 0.1, 2.0));
	vertices.push_back(glm::vec3(-0.1, 0.0, 2.0));
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
	_light.SetLightColor({1, 1, 1});
	_light.SetLightAngle(30);
	_light.SetLightAngleFade(10);

	_video->RegisterLight(&_light);
	_video->GetInputControl()->Subscribe(this);
}

Player::~Player()
{
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
	} else if (key == GLFW_KEY_W) {
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
				_ship->StopPreview();
				_buildMode = false;
				Logger::Verbose() << "Build mode off.";
			} else {
				_buildMode = true;
				_buildPos = {0, 0, 0};
				_ship->PreviewBlock(_buildPos);
				Logger::Verbose() << "Build mode on.";
			}
		}
	}

	if (_buildMode) {
		BuildActions(key, action);
	}
}

void Player::BuildActions(int key, int action)
{
	if (key == GLFW_KEY_I) {
		if (action == GLFW_PRESS) {
			++_buildPos.x;
			_ship->PreviewBlock(_buildPos);
		}
	} else if (key == GLFW_KEY_K) {
		if (action == GLFW_PRESS) {
			--_buildPos.x;
			_ship->PreviewBlock(_buildPos);
		}
	} else if (key == GLFW_KEY_L) {
		if (action == GLFW_PRESS) {
			--_buildPos.y;
			_ship->PreviewBlock(_buildPos);
		}
	} else if (key == GLFW_KEY_J) {
		if (action == GLFW_PRESS) {
			++_buildPos.y;
			_ship->PreviewBlock(_buildPos);
		}
	} else if (key == GLFW_KEY_Y) {
		if (action == GLFW_PRESS) {
			++_buildPos.z;
			_ship->PreviewBlock(_buildPos);
		}
	} else if (key == GLFW_KEY_H) {
		if (action == GLFW_PRESS) {
			--_buildPos.z;
			_ship->PreviewBlock(_buildPos);
		}
	} else if (key == GLFW_KEY_P) {
		if (action == GLFW_PRESS) {
			_ship->InsertBlock(_buildPos);
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

void Player::Tick()
{
	_mutex.lock();
	glm::vec2 hdir(
		sinf(glm::radians(_angleH)),
		cosf(glm::radians(_angleH)));

	glm::vec2 hdirStrafe(
		sinf(glm::radians(_angleH + 90)),
		cosf(glm::radians(_angleH + 90)));

	glm::vec2 hspeed = hdir * (float)_go +
		hdirStrafe * (float)_strafe;

	_vspeed -= 0.098 / 2;

	glm::vec3 effect = GetObjectEffect();

	if (effect.z > 0 && _vspeed < 0) {
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
	}

	if (effect.z > 0 && _jump) {
		_vspeed = 6;
	}

	if (fabs(effect.x) + fabs(effect.y) > 0.0001) {
		if (effect.x * hspeed.x + effect.y * hspeed.y < 0) {
			hspeed = glm::vec2(effect.x, effect.y);
		}
	}

	_pos.x += hspeed.x / 20;
	_pos.y += hspeed.y / 20;
	//_pos.z += _vspeed / 200;

	SetObjectMatrix(
		glm::rotate(glm::translate(glm::mat4(1.0), _pos),
			glm::radians(_angleH), glm::vec3(0, 0, 1)));
	SetObjectSpeed(glm::vec3(
		hspeed.x / 20,
		hspeed.y / 20,
		_vspeed / 200));

	_video->SetCameraPosition(_pos + glm::vec3(0, 0, 1.85));
	_video->SetCameraDirection(glm::vec3(
			hdir * cosf(glm::radians(_angleV)),
			sinf(glm::radians(_angleV))));

	_light.SetLightPosition(_pos + glm::vec3(0, 0, 1.2) +
			glm::vec3(-hdirStrafe * 0.3f, 0.0f));
	_light.SetLightDirection(glm::vec3(
			hdir * cosf(glm::radians(_angleV)),
			sinf(glm::radians(_angleV))));
	_mutex.unlock();

	_rayEngine->RayCast(
		_pos + glm::vec3(0, 0, 1.85),
		glm::vec3(
			hdir * cosf(glm::radians(_angleV)),
			sinf(glm::radians(_angleV))),
		5,
		nullptr);
}