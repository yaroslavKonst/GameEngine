#include "ship.h"

#include "../Utils/loader.h"

#include "../Logger/logger.h"

Ship::Ship(Video* video, CollisionEngine* collisionEngine)
{
	_video = video;
	_collisionEngine = collisionEngine;

	_baseGrid = new BaseGrid(_video, _collisionEngine);
	_mainGrid = new MainGrid(_video, _collisionEngine, _baseGrid);

	_baseGrid->InsertBlock(0, 0, BaseBlock::Type::Floor, true);

	_video->GetInputControl()->Subscribe(this);

	_buildX = 0;
	_buildY = 0;
	_buildType = BaseBlock::Type::Floor;
	_mainBuildType = MainBlock::Type::Wall;
	_buildLayer = 0;
	_prevBuildLayer = 100;
}

Ship::~Ship()
{
	_video->GetInputControl()->UnSubscribe(this);

	delete _mainGrid;
	delete _baseGrid;
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
				0,
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
				0,
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
				0,
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
				0,
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
				0,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
			_mainGrid->InsertBlock(
				_buildX,
				_buildY,
				0,
				_mainBuildType);
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				0,
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
				0,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_T) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
			_mainBuildType = MainBlock::Type::Wall;
			_mainGrid->PreviewBlock(
				_buildX,
				_buildY,
				0,
				_mainBuildType);
			_video->UnlockSceneMutex();
		}
	}
}
