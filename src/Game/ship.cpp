#include "ship.h"

#include "../Utils/loader.h"

#include "../Logger/logger.h"

Ship::Ship(Video* video, CollisionEngine* collisionEngine)
{
	_video = video;
	_collisionEngine = collisionEngine;

	_baseGrid = new BaseGrid(_video, _collisionEngine, &_shipMatrix);
	_mainGrid = new MainGrid(
		_video,
		_collisionEngine,
		_baseGrid,
		&_shipMatrix);

	_baseGrid->InsertBlock(0, 0, BaseBlock::Type::Floor, true);

	SetInputLayer(0);
	InputArea = {-1, -1, 1, 1};

	_video->GetInputControl()->Subscribe(this);

	_buildX = 0;
	_buildY = 0;
	_buildRotation = 0;
	_buildType = BaseBlock::Type::Floor;
	_mainBuildType = MainBlock::Type::Wall;
	_buildLayer = 0;
	_prevBuildLayer = 100;

	_position = {0, 0, 0};
	_speed = {0, 0, 0.001};
	_force = {0, 0, 0};
}

Ship::~Ship()
{
	_video->GetInputControl()->UnSubscribe(this);

	delete _mainGrid;
	delete _baseGrid;
}

void Ship::TickEarly()
{
	_speed += _force;
	_position += _speed;

	_shipMatrix = glm::translate(
		glm::mat4(1.0),
		_position);
}

void Ship::Tick()
{
}

void Ship::Key(int key, int scancode, int action, int mods)
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
			_video->LockSceneMutex();
			_baseGrid->StopPreview();
			_mainGrid->StopPreview();
			_video->UnlockSceneMutex();
			SetInputEnabled(false);
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
			_video->LockSceneMutex();
			_video->UnlockSceneMutex();
			break;
		case 1:
			_mainGrid->StopPreview();
			_video->LockSceneMutex();
			_video->UnlockSceneMutex();
			break;
		default:
			break;
		}

		switch (_buildLayer) {
		case 0:
			_video->LockSceneMutex();
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
			break;
		case 1:
			_video->LockSceneMutex();
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			_video->UnlockSceneMutex();
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
			_video->LockSceneMutex();
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) {
			--_buildX;
			_video->LockSceneMutex();
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			--_buildY;
			_video->LockSceneMutex();
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			++_buildY;
			_video->LockSceneMutex();
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
			_baseGrid->InsertBlock(_buildX, _buildY, _buildType);
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
			_baseGrid->RemoveBlock(_buildX, _buildY);
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_T) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
			_buildType = _buildType == BaseBlock::Type::Floor ?
				BaseBlock::Type::FloorComm :
				BaseBlock::Type::Floor;
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
		}
	}
}

void Ship::MainLayer(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) {
			++_buildX;
			_video->LockSceneMutex();
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) {
			--_buildX;
			_video->LockSceneMutex();
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			--_buildY;
			_video->LockSceneMutex();
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			++_buildY;
			_video->LockSceneMutex();
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
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
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
			_mainGrid->RemoveBlock(_buildX, _buildY);
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_T) {
		if (action == GLFW_PRESS) {
			switch (_mainBuildType) {
			case MainBlock::Type::Wall:
				_mainBuildType = MainBlock::Type::FlightControl;
				break;
			case MainBlock::Type::FlightControl:
				_mainBuildType = MainBlock::Type::Wall;
				break;
			default:
				break;
			}

			_video->LockSceneMutex();
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				_buildRotation,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	}
}

bool Ship::Scroll(double xoffset, double yoffset)
{
	if (_buildLayer != 1) {
		return true;
	}

	if (yoffset > 0) {
		_buildRotation += 10;
	} else {
		_buildRotation -= 10;
	}

	if (_buildRotation >= 360) {
		_buildRotation = 0;
	} else if (_buildRotation < 0) {
		_buildRotation = 350;
	}

	_video->LockSceneMutex();
	_mainGrid->PreviewBlock(
		_buildX,
		_buildY,
		_buildRotation,
		_mainBuildType);
	_video->UnlockSceneMutex();

	return true;
}
