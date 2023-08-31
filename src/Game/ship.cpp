#include "ship.h"

#include "../Utils/loader.h"

Ship::Ship(Video* video, CollisionEngine* collisionEngine)
{
	_video = video;
	_collisionEngine = collisionEngine;

	_baseGrid = new BaseGrid(_video, _collisionEngine);

	_baseGrid->InsertBlock(0, 0, BaseBlock::Type::Floor, true);

	_video->GetInputControl()->Subscribe(this);

	_buildX = 0;
	_buildY = 0;
	_buildType = BaseBlock::Type::Floor;
}

Ship::~Ship()
{
	_video->GetInputControl()->UnSubscribe(this);

	delete _baseGrid;
}

void Ship::Tick()
{
}

void Ship::Key(int key, int scancode, int action, int mods)
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
	} else if (key == GLFW_KEY_R) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
			_buildType = _buildType == BaseBlock::Type::Floor ?
				BaseBlock::Type::FloorComm :
				BaseBlock::Type::Floor;
			_baseGrid->PreviewBlock(_buildX, _buildY, _buildType);
			_video->UnlockSceneMutex();
		}
	} else if (key == GLFW_KEY_B) {
		if (action == GLFW_PRESS) {
			_video->LockSceneMutex();
			_baseGrid->StopPreview();
			_video->UnlockSceneMutex();
			SetInputEnabled(false);
		}
	}
}

BaseGrid::BaseGrid(Video* video, CollisionEngine* collisionEngine)
{
	_video = video;
	_collisionEngine = collisionEngine;

	int tw;
	int th;
	auto td = Loader::LoadImage("Models/Ship/ShipFloor.png", tw, th);
	_floorTexture = _video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipFloorComm.png", tw, th);
	_floorCommTexture = _video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipFloorCommDelim.png", tw, th);
	_floorCommDelimTexture = _video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipPowerCableHub.png", tw, th);
	_powerCableHubTexture = _video->GetTextures()->AddTexture(tw, th, td);

	_preview = nullptr;
}

BaseGrid::~BaseGrid()
{
	for (auto& block : _blocks) {
		_video->RemoveModel(block.second);
		delete block.second;
	}

	StopPreview();

	_video->GetTextures()->RemoveTexture(_floorTexture);
	_video->GetTextures()->RemoveTexture(_floorCommTexture);
	_video->GetTextures()->RemoveTexture(_floorCommDelimTexture);
	_video->GetTextures()->RemoveTexture(_powerCableHubTexture);
}

void BaseGrid::InsertBlock(
	int32_t x,
	int32_t y,
	BaseBlock::Type type,
	bool force)
{
	bool available =
		_blocks.find({x, y}) == _blocks.end() &&
		(_blocks.find({x - 1, y}) != _blocks.end() ||
		_blocks.find({x + 1, y}) != _blocks.end() ||
		_blocks.find({x, y - 1}) != _blocks.end() ||
		_blocks.find({x, y + 1}) != _blocks.end());

	available = available || force;

	if (!available) {
		return;
	}

	BaseBlock* block;

	switch (type) {
	case BaseBlock::Type::Floor:
		block = new FloorBlock(_floorTexture);
		break;
	case BaseBlock::Type::FloorComm:
		block = new FloorCommBlock(
			_floorCommTexture,
			_video,
			_floorCommDelimTexture,
			_powerCableHubTexture);
		break;
	default:
		return;
	}

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(x, y, 0));

	block->SetDrawEnabled(true);
	block->SetModelMatrix(matrix);

	block->SetModelInnerMatrix(glm::mat4(1.0));
	block->SetModelInstances({glm::mat4(1.0)});

	auto model = Loader::LoadModel("Models/Ship/ShipFloorCollision.obj");
	block->SetObjectVertices(model.Vertices);
	block->SetObjectIndices(model.Indices);
	block->SetObjectCenter();
	block->SetObjectMatrix(matrix);
	block->SetObjectDynamic(true);
	block->SetObjectDomain(1);

	_video->RegisterModel(block);
	_collisionEngine->RegisterObject(block);

	_blocks[{x, y}] = block;

	for (auto& block : _blocks) {
		block.second->Update(this, block.first.X, block.first.Y);
	}
}

void BaseGrid::RemoveBlock(int32_t x, int32_t y)
{
	if (_blocks.find({x, y}) == _blocks.end()) {
		return;
	}

	_video->RemoveModel(_blocks[{x, y}]);
	_collisionEngine->RemoveObject(_blocks[{x, y}]);

	delete _blocks[{x, y}];
	_blocks.erase({x, y});

	for (auto& block : _blocks) {
		block.second->Update(this, block.first.X, block.first.Y);
	}
}

void BaseGrid::PreviewBlock(int32_t x, int32_t y, BaseBlock::Type type)
{
	StopPreview();

	switch (type) {
	case BaseBlock::Type::Floor:
		_preview = new FloorBlock(_floorTexture);
		break;
	case BaseBlock::Type::FloorComm:
		_preview = new FloorCommBlock(
			_floorCommTexture,
			_video,
			_floorCommDelimTexture,
			_powerCableHubTexture);
		break;
	default:
		return;
	}

	bool available =
		_blocks.find({x, y}) == _blocks.end() &&
		(_blocks.find({x - 1, y}) != _blocks.end() ||
		_blocks.find({x + 1, y}) != _blocks.end() ||
		_blocks.find({x, y - 1}) != _blocks.end() ||
		_blocks.find({x, y + 1}) != _blocks.end());

	_preview->SetDrawEnabled(true);
	_preview->SetDrawLight(true);
	if (available) {
		_preview->SetColorMultiplier({0.5, 1, 0.5, 0.4});
	} else {
		_preview->SetColorMultiplier({1.0, 0.5, 0.5, 0.4});
	}
	_preview->SetModelMatrix(glm::translate(
		glm::mat4(1.0),
		glm::vec3(x, y, 0)));

	_preview->SetModelInnerMatrix(glm::scale(
		glm::mat4(1.0),
		glm::vec3(1,1,1.1)));
	_preview->SetModelInstances({glm::mat4(1.0)});

	_video->RegisterModel(_preview);
}

void BaseGrid::StopPreview()
{
	if (_preview) {
		_video->RemoveModel(_preview);
		delete _preview;
		_preview = nullptr;
	}
}

BaseBlock::Type BaseGrid::GetType(int32_t x, int32_t y)
{
	if (_blocks.find({x, y}) == _blocks.end()) {
		return BaseBlock::Type::Empty;
	}

	return _blocks[{x, y}]->GetType();
}

BaseBlock* BaseGrid::GetBlock(int32_t x, int32_t y)
{
	if (_blocks.find({x, y}) == _blocks.end()) {
		return nullptr;
	}

	return _blocks[{x, y}];
}

FloorBlock::FloorBlock(uint32_t texture)
{
	SetTexture({texture});

	auto model = Loader::LoadModel("Models/Ship/ShipFloor.obj");
	SetModelVertices(model.Vertices);
	SetModelTexCoords(model.TexCoords);
	SetModelIndices(model.Indices);
	SetModelNormals(model.Normals);
}

FloorCommBlock::FloorCommBlock(
	uint32_t texture,
	Video* video,
	uint32_t delimTexture,
	uint32_t powerCableHubTexture)
{
	_video = video;
	SetTexture({texture});
	_delims.resize(4, nullptr);
	_powerCables.resize(4, nullptr);
	_powerCableHub = nullptr;

	_delimTexture = delimTexture;
	_powerCableHubTexture = powerCableHubTexture;

	auto model = Loader::LoadModel("Models/Ship/ShipFloorComm.obj");
	SetModelVertices(model.Vertices);
	SetModelTexCoords(model.TexCoords);
	SetModelIndices(model.Indices);
	SetModelNormals(model.Normals);

	_hasPowerCable = false;
}

FloorCommBlock::~FloorCommBlock()
{
	for (auto delim : _delims) {
		if (delim) {
			_video->RemoveModel(delim);
			delete delim;
		}
	}

	for (auto cable : _powerCables) {
		if (cable) {
			_video->RemoveModel(cable);
			delete cable;
		}
	}

	if (_powerCableHub) {
		_video->RemoveModel(_powerCableHub);
		delete _powerCableHub;
		_powerCableHub = nullptr;
	}
}

void FloorCommBlock::Update(BaseGrid* grid, int32_t x, int32_t y)
{
	std::vector<bool> currentState(4);
	std::vector<bool> newState(4);

	newState[0] = grid->GetType(x - 1, y) != BaseBlock::Type::FloorComm;
	newState[1] = grid->GetType(x + 1, y) != BaseBlock::Type::FloorComm;
	newState[2] = grid->GetType(x, y - 1) != BaseBlock::Type::FloorComm;
	newState[3] = grid->GetType(x, y + 1) != BaseBlock::Type::FloorComm;

	currentState[0] = _delims[0] != nullptr;
	currentState[1] = _delims[1] != nullptr;
	currentState[2] = _delims[2] != nullptr;
	currentState[3] = _delims[3] != nullptr;

	if (currentState == newState) {
		return;
	}

	for (auto delim : _delims) {
		if (delim) {
			_video->RemoveModel(delim);
			delete delim;
		}
	}

	for (size_t i = 0; i < _delims.size(); ++i) {
		if (!newState[i]) {
			_delims[i] = nullptr;
			continue;
		}

		_delims[i] = new Model;

		auto model = Loader::LoadModel(
			"Models/Ship/ShipFloorCommDelim.obj");
		_delims[i]->SetModelVertices(model.Vertices);
		_delims[i]->SetModelTexCoords(model.TexCoords);
		_delims[i]->SetModelIndices(model.Indices);
		_delims[i]->SetModelNormals(model.Normals);
		_delims[i]->SetTexture({_delimTexture});

		glm::mat4 matrix = glm::translate(
			glm::mat4(1.0),
			glm::vec3(x, y, 0.8));

		float rotation = 0;

		switch (i) {
		case 0:
			rotation = 180;
			break;
		case 1:
			rotation = 0;
			break;
		case 2:
			rotation = 270;
			break;
		case 3:
			rotation = 90;
			break;
		}

		matrix = glm::rotate(
			matrix,
			glm::radians(rotation),
			glm::vec3(0, 0, 1));
		matrix = glm::translate(
			matrix,
			glm::vec3(0.4, 0, 0));

		_delims[i]->SetDrawEnabled(true);
		_delims[i]->SetModelMatrix(matrix);

		_delims[i]->SetModelInnerMatrix(glm::mat4(1.0));
		_delims[i]->SetModelInstances({glm::mat4(1.0)});

		_video->RegisterModel(_delims[i]);
	}
}

bool FloorCommBlock::GetPowerCable()
{
	return _hasPowerCable;
}

void FloorCommBlock::SetPowerCable(bool value)
{
	_hasPowerCable = value;

	if (value && !_powerCableHub) {
		_powerCableHub = new Model;

		auto model = Loader::LoadModel(
			"Models/Ship/ShipPowerCableHub.obj");
		_powerCableHub->SetModelVertices(model.Vertices);
		_powerCableHub->SetModelTexCoords(model.TexCoords);
		_powerCableHub->SetModelIndices(model.Indices);
		_powerCableHub->SetModelNormals(model.Normals);
		_powerCableHub->SetTexture({_delimTexture});
		_powerCableHub->SetColorMultiplier({1, 0.8, 0.8, 1});

		glm::mat4 matrix = GetModelMatrix();

		matrix = glm::translate(
			matrix,
			glm::vec3(0, 0, 0.8));

		_powerCableHub->SetDrawEnabled(true);
		_powerCableHub->SetModelMatrix(matrix);

		_powerCableHub->SetModelInnerMatrix(glm::mat4(1.0));
		_powerCableHub->SetModelInstances({glm::mat4(1.0)});

		_video->RegisterModel(_powerCableHub);
	}

	if (!value && _powerCableHub) {
		_video->RemoveModel(_powerCableHub);
		delete _powerCableHub;
		_powerCableHub = nullptr;
	}
}
