#include "ship.h"

#include "../Utils/loader.h"

#include "../Logger/logger.h"

Ship::Ship(Video* video, CollisionEngine* collisionEngine)
{
	_video = video;
	_collisionEngine = collisionEngine;

	_shipMatrix = glm::mat4(1.0);

	_baseGrid = new BaseGrid(_video, _collisionEngine, &_shipMatrix);
	_mainGrid = new MainGrid(
		_video,
		_collisionEngine,
		_baseGrid,
		&_shipMatrix);

	_baseGrid->InsertBlock(0, 0, BaseBlock::Type::Floor, true);

	SetInputLayer(1);
	InputArea = {-1, -1, 1, 1};

	_video->GetInputControl()->Subscribe(this);

	_flightMode = false;
	_buildMode = false;

	_buildX = 0;
	_buildY = 0;
	_buildRotation = 0;
	_buildRotationStep = 90;
	_buildType = BaseBlock::Type::Floor;
	_mainBuildType = MainBlock::Type::Wall;
	_buildLayer = 0;
	_prevBuildLayer = 100;

	_grounded = false;

	_position = {0, 0, 0};
	_linearSpeed = {0, 0, 0};
	_rotation = {0, 0, 0};
	_angularSpeed = {0, 0, 0};

	_targetSpeed = {0, 0, 0};
	_rotationMatrix = glm::mat4(1.0f);

	_controlF = 0;
	_controlR = 0;
	_controlU = 0;
}

Ship::~Ship()
{
	_video->GetInputControl()->Unsubscribe(this);

	delete _mainGrid;
	delete _baseGrid;
}

void Ship::TickEarly()
{
	if (_flightMode) {
		glm::vec3 dirF = _shipMatrix *
			_activeFlightControl->ModelParams.Matrix *
			glm::vec4(0, 1, 0, 0);
		glm::vec3 dirR = _shipMatrix *
			_activeFlightControl->ModelParams.Matrix *
			glm::vec4(1, 0, 0, 0);
		glm::vec3 dirU = _shipMatrix *
			_activeFlightControl->ModelParams.Matrix *
			glm::vec4(0, 0, 1, 0);

		_targetSpeed =
			dirF * _controlF +
			dirR * _controlR +
			dirU * _controlU;
	} else {
		_targetSpeed = {0, 0, 0};
	}

	glm::vec3 mg(0, 0, -1);

	glm::vec3 thrusterForce;
	glm::vec3 targetForce = _targetSpeed / 10.0f - _linearSpeed;

	if (!_flightMode && _grounded) {
		thrusterForce = glm::vec3(0, 0, 0);
	} else {
		thrusterForce = _mainGrid->SetThrusterForce(targetForce - mg);
		_grounded = false;
	}

	glm::vec3 force = mg + thrusterForce;

	_linearSpeed += force / 100.0f;
	_position += _linearSpeed;

	_shipMatrix = glm::translate(
		glm::mat4(1.0f),
		_position);

	_shipMatrix = _shipMatrix * _rotationMatrix;
}

void Ship::Tick()
{
	glm::vec3 effect = _baseGrid->GetCollisionFeedback();

	_position += effect;

	if (glm::dot(effect, _linearSpeed) < 0) {
		_linearSpeed = glm::vec3(0, 0, 0);
		_grounded = true;
	}
}

void Ship::Key(int key, int scancode, int action, int mods)
{
	if (_flightMode) {
		Flight(key, scancode, action, mods);
	} else if (_buildMode) {
		Build(key, scancode, action, mods);
	}
}

void Ship::Build(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R) {
		if (action == GLFW_PRESS) {
			++_buildLayer;
		}
	} else if (key == GLFW_KEY_F) {
		if (action == GLFW_PRESS) {
			--_buildLayer;
		}
	} else if (key == GLFW_KEY_B) {
		if (action == GLFW_PRESS) {
			_baseGrid->StopPreview();
			_mainGrid->StopPreview();
			SetInputEnabled(false);
			_buildMode = false;
			_prevBuildLayer = 100;
			return;
		}
	}

	if (_buildLayer < 0) {
		_buildLayer = 0;
	}

	if (_buildLayer > 1) {
		_buildLayer = 1;
	}

	if (_prevBuildLayer != _buildLayer) {
		switch (_prevBuildLayer) {
		case 0:
			_baseGrid->StopPreview();
			break;
		case 1:
			_mainGrid->StopPreview();
			break;
		default:
			break;
		}

		switch (_buildLayer) {
		case 0:
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			break;
		case 1:
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			break;
		default:
			break;
		}
	}

	_prevBuildLayer = _buildLayer;

	switch (_buildLayer) {
	case 0:
		BaseLayer(key, scancode, action, mods);
		break;
	case 1:
		MainLayer(key, scancode, action, mods);
		break;
	default:
		break;
	}
}

void Ship::BaseLayer(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) {
			++_buildX;
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
		}
	} else if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) {
			--_buildX;
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
		}
	} else if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			--_buildY;
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
		}
	} else if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			++_buildY;
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
		}
	} else if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS) {
			_baseGrid->InsertBlock(_buildX, _buildY, _buildType);
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
		}
	} else if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS) {
			_baseGrid->RemoveBlock(_buildX, _buildY);
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
		}
	} else if (key == GLFW_KEY_T) {
		if (action == GLFW_PRESS) {
			_buildType = _buildType == BaseBlock::Type::Floor ?
				BaseBlock::Type::FloorComm :
				BaseBlock::Type::Floor;
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);

			_buildRotation = 0;
		}
	}
}

void Ship::MainLayer(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) {
			++_buildX;
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
		}
	} else if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) {
			--_buildX;
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
		}
	} else if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			--_buildY;
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
		}
	} else if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			++_buildY;
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
		}
	} else if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS) {
			_mainGrid->InsertBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
		}
	} else if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS) {
			_mainGrid->RemoveBlock(_buildX, _buildY);
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
		}
	} else if (key == GLFW_KEY_T) {
		if (action == GLFW_PRESS) {
			_buildRotation = 0;

			switch (_mainBuildType) {
			case MainBlock::Type::Wall:
				_mainBuildType =
					MainBlock::Type::StaticThruster;
				_buildRotationStep = 90;
				break;
			case MainBlock::Type::StaticThruster:
				_mainBuildType =
					MainBlock::Type::DynamicThruster;
				_buildRotationStep = 90;
				break;
			case MainBlock::Type::DynamicThruster:
				_mainBuildType = MainBlock::Type::FlightControl;
				_buildRotationStep = 5;
				break;
			case MainBlock::Type::FlightControl:
				_mainBuildType = MainBlock::Type::Wall;
				_buildRotationStep = 90;
				break;
			default:
				break;
			}

			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
		}
	}
}

bool Ship::MouseMoveRaw(double xoffset, double yoffset)
{
	if (!_flightMode) {
		return false;
	}

	glm::vec3 dirU = _shipMatrix *
		_activeFlightControl->ModelParams.Matrix *
		glm::vec4(0, 0, 1, 0);

	_rotationMatrix = glm::rotate(
		_rotationMatrix,
		glm::radians((float)xoffset),
		dirU);

	return true;
}

bool Ship::Scroll(double xoffset, double yoffset)
{
	if (_buildLayer != 1) {
		return true;
	}

	if (yoffset > 0) {
		_buildRotation += _buildRotationStep;
	} else {
		_buildRotation -= _buildRotationStep;
	}

	if (_buildRotation >= 360) {
		_buildRotation = 0;
	} else if (_buildRotation < 0) {
		_buildRotation = 360 - _buildRotationStep;
	}

	_mainGrid->PreviewBlock(
		_buildX,
		_buildY,
		_buildRotation,
		_mainBuildType);

	return true;
}

void Ship::Flight(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) {
			_controlF += 1;
		} else if (action == GLFW_RELEASE) {
			_controlF -= 1;
		}
	} else if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) {
			_controlF -= 1;
		} else if (action == GLFW_RELEASE) {
			_controlF += 1;
		}
	} else if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			_controlR += 1;
		} else if (action == GLFW_RELEASE) {
			_controlR -= 1;
		}
	} else if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			_controlR -= 1;
		} else if (action == GLFW_RELEASE) {
			_controlR += 1;
		}
	} else if (key == GLFW_KEY_SPACE) {
		if (action == GLFW_PRESS) {
			_controlU += 1;
		} else if (action == GLFW_RELEASE) {
			_controlU -= 1;
		}
	} else if (key == GLFW_KEY_X) {
		if (action == GLFW_PRESS) {
			_controlU -= 1;
		} else if (action == GLFW_RELEASE) {
			_controlU += 1;
		}
	} else if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS) {
			_targetSpeed = glm::vec3(0.0);
			SetInputEnabled(false);
			_flightMode = false;
			return;
		}
	}
}
