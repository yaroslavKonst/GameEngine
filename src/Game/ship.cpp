#include "ship.h"

#include "../Utils/loader.h"

#include "../Logger/logger.h"

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
	Textures["Floor"] = _video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipFloorComm.png", tw, th);
	Textures["FloorComm"] = _video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipFloorCommDelim.png", tw, th);
	Textures["FloorCommDelim"] =
		_video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipPowerCableHub.png", tw, th);
	Textures["PowerCableHub"] =
		_video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipPowerCable.png", tw, th);
	Textures["PowerCable"] = _video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipDataCable.png", tw, th);
	Textures["DataCable"] = _video->GetTextures()->AddTexture(tw, th, td);

	td = Loader::LoadImage("Models/Ship/ShipDataCableHub.png", tw, th);
	Textures["DataCableHub"] =
		_video->GetTextures()->AddTexture(tw, th, td);

	_preview = nullptr;

	auto model = Loader::LoadModel("Models/Ship/ShipFloor.obj");
	Models["Floor"] = _video->LoadModel(model);

	model = Loader::LoadModel("Models/Ship/ShipFloorComm.obj");
	Models["FloorComm"] = _video->LoadModel(model);

	model = Loader::LoadModel("Models/Ship/ShipFloorCommDelim.obj");
	Models["FloorCommDelim"] = _video->LoadModel(model);

	model = Loader::LoadModel("Models/Ship/ShipPowerCable.obj");
	Models["PowerCable"] = _video->LoadModel(model);

	model = Loader::LoadModel("Models/Ship/ShipPowerCableHub.obj");
	Models["PowerCableHub"] = _video->LoadModel(model);

	model = Loader::LoadModel("Models/Ship/ShipDataCable.obj");
	Models["DataCable"] = _video->LoadModel(model);

	model = Loader::LoadModel("Models/Ship/ShipDataCableHub.obj");
	Models["DataCableHub"] = _video->LoadModel(model);
}

BaseGrid::~BaseGrid()
{
	for (auto& block : _blocks) {
		_video->RemoveModel(block.second);
		delete block.second;
	}

	StopPreview();

	for (auto& texture : Textures) {
		_video->GetTextures()->RemoveTexture(texture.second);
	}

	for (auto& model : Models) {
		_video->UnloadModel(model.second);
	}
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
		block = new FloorBlock(x, y, this);
		break;
	case BaseBlock::Type::FloorComm:
		block = new FloorCommBlock(
			x,
			y,
			this,
			_video);
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

	auto model = Loader::LoadModel("Models/Ship/ShipFloorCollision.obj");
	block->SetObjectVertices(model.Vertices);
	block->SetObjectIndices(model.Indices);
	block->SetObjectNormals(model.Normals);
	block->SetObjectCenter();
	block->SetObjectMatrix(matrix);
	block->SetObjectDynamic(true);
	block->SetObjectSphereCenter({0, 0, 0.5});
	block->SetObjectSphereRadius(0.5);
	block->SetObjectDomain(1);

	_video->RegisterModel(block);
	_collisionEngine->RegisterObject(block);

	_blocks[{x, y}] = block;

	for (auto& block : _blocks) {
		block.second->Update();
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
		block.second->Update();
	}
}

void BaseGrid::PreviewBlock(int32_t x, int32_t y, BaseBlock::Type type)
{
	StopPreview();

	switch (type) {
	case BaseBlock::Type::Floor:
		_preview = new FloorBlock(x, y, this);
		break;
	case BaseBlock::Type::FloorComm:
		_preview = new FloorCommBlock(
			x,
			y,
			this,
			_video);
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

FloorBlock::FloorBlock(
	int32_t x,
	int32_t y,
	BaseGrid* grid) :
	BaseBlock(x, y, grid)
{
	SetTexture({grid->Textures["Floor"]});
	SetModels({grid->Models["Floor"]});
}

FloorCommBlock::FloorCommBlock(
	int32_t x,
	int32_t y,
	BaseGrid* grid,
	Video* video) :
	BaseBlock(x, y, grid)
{
	_video = video;
	SetTexture({grid->Textures["FloorComm"]});
	SetModels({grid->Models["FloorComm"]});
	_delims.resize(4, nullptr);
	_powerCables.resize(4, nullptr);
	_dataCables.resize(4, nullptr);
	_powerCableHub = nullptr;
	_dataCableHub = nullptr;

	_hasPowerCable = false;
	_hasDataCable = false;
	_forceUpdate = false;

	for (size_t i = 0; i < 4; ++i) {
		_delims[i] = CreateDelim(i);
		_delims[i]->SetDrawEnabled(true);
		_delims[i]->SetTexture({grid->Textures["FloorCommDelim"]});
		_delims[i]->SetModels({grid->Models["FloorCommDelim"]});

		_powerCables[i] = CreatePowerCable(i);
		_powerCables[i]->SetDrawEnabled(false);
		_powerCables[i]->SetTexture({grid->Textures["PowerCable"]});
		_powerCables[i]->SetModels({grid->Models["PowerCable"]});

		_dataCables[i] = CreateDataCable(i);
		_dataCables[i]->SetDrawEnabled(false);
		_dataCables[i]->SetTexture({grid->Textures["DataCable"]});
		_dataCables[i]->SetModels({grid->Models["DataCable"]});

		_video->RegisterModel(_delims[i]);
		_video->RegisterModel(_powerCables[i]);
		_video->RegisterModel(_dataCables[i]);
	}
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

	for (auto cable : _dataCables) {
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

void FloorCommBlock::Update()
{
	bool power;
	bool data;
	bool commBlock;

	std::vector<int32_t> x = {_x - 1, _x + 1, _x, _x};
	std::vector<int32_t> y = {_y, _y, _y - 1, _y + 1};

	for (uint32_t i = 0; i < 4; ++i) {
		commBlock = _grid->GetType(x[i], y[i]) ==
			BaseBlock::Type::FloorComm;

		if (!commBlock) {
			_delims[i]->SetDrawEnabled(true);
			_powerCables[i]->SetDrawEnabled(false);
			_dataCables[i]->SetDrawEnabled(false);
		} else {
			power = static_cast<FloorCommBlock*>(
				_grid->GetBlock(x[i], y[i]))->GetPowerCable();
			data = static_cast<FloorCommBlock*>(
				_grid->GetBlock(x[i], y[i]))->GetDataCable();

			power = power && _hasPowerCable;
			data = data && _hasDataCable;

			_delims[i]->SetDrawEnabled(!(power || data));
			_powerCables[i]->SetDrawEnabled(power);
			_dataCables[i]->SetDrawEnabled(data);
		}
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

		_powerCableHub->SetModels({_grid->Models["PowerCableHub"]});
		_powerCableHub->SetTexture({_grid->Textures["PowerCableHub"]});
		_powerCableHub->SetColorMultiplier({1, 0.8, 0.8, 1});

		glm::mat4 matrix = GetModelMatrix();

		matrix = glm::translate(
			matrix,
			glm::vec3(0, 0, 0.8));

		_powerCableHub->SetDrawEnabled(true);
		_powerCableHub->SetModelMatrix(matrix);

		_powerCableHub->SetModelInnerMatrix(glm::mat4(1.0));

		_video->RegisterModel(_powerCableHub);

		Update();

		BaseBlock* block = _grid->GetBlock(_x - 1, _y);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x + 1, _y);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x, _y - 1);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x, _y + 1);
		if (block) {
			block->Update();
		}
	}

	if (!value && _powerCableHub) {
		_video->RemoveModel(_powerCableHub);
		delete _powerCableHub;
		_powerCableHub = nullptr;

		Update();

		BaseBlock* block = _grid->GetBlock(_x - 1, _y);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x + 1, _y);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x, _y - 1);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x, _y + 1);
		if (block) {
			block->Update();
		}
	}
}

bool FloorCommBlock::GetDataCable()
{
	return _hasDataCable;
}

void FloorCommBlock::SetDataCable(bool value)
{
	_hasDataCable = value;

	if (value && !_dataCableHub) {
		_dataCableHub = new Model;

		_dataCableHub->SetModels({_grid->Models["DataCableHub"]});
		_dataCableHub->SetTexture({_grid->Textures["DataCableHub"]});
		_dataCableHub->SetColorMultiplier({0.8, 0.8, 1, 1});

		glm::mat4 matrix = GetModelMatrix();

		matrix = glm::translate(
			matrix,
			glm::vec3(0, 0, 0.8));

		_dataCableHub->SetDrawEnabled(true);
		_dataCableHub->SetModelMatrix(matrix);

		_dataCableHub->SetModelInnerMatrix(glm::mat4(1.0));

		_video->RegisterModel(_dataCableHub);

		Update();

		BaseBlock* block = _grid->GetBlock(_x - 1, _y);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x + 1, _y);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x, _y - 1);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x, _y + 1);
		if (block) {
			block->Update();
		}
	}

	if (!value && _dataCableHub) {
		_video->RemoveModel(_dataCableHub);
		delete _dataCableHub;
		_dataCableHub = nullptr;

		Update();

		BaseBlock* block = _grid->GetBlock(_x - 1, _y);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x + 1, _y);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x, _y - 1);
		if (block) {
			block->Update();
		}

		block = _grid->GetBlock(_x, _y + 1);
		if (block) {
			block->Update();
		}
	}
}

Model* FloorCommBlock::CreateDelim(size_t i)
{
	Model* delim = new Model;

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(_x, _y, 0.8));

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

	delim->SetModelMatrix(matrix);

	delim->SetModelInnerMatrix(glm::mat4(1.0));

	return delim;
}

Model* FloorCommBlock::CreatePowerCable(size_t i)
{
	Model* cable = new Model;

	cable->SetColorMultiplier({1, 0.8, 0.8, 1});

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(_x, _y, 0.84));

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
		glm::vec3(0.25, 0, 0));
	matrix = glm::rotate(
		matrix,
		glm::radians(90.0f),
		glm::vec3(0, 0, 1));

	cable->SetModelMatrix(matrix);

	cable->SetModelInnerMatrix(glm::mat4(1.0));

	return cable;
}

Model* FloorCommBlock::CreateDataCable(size_t i)
{
	Model* cable = new Model;

	cable->SetColorMultiplier({0.8, 0.8, 1, 1});

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(_x, _y, 0.84));

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
		glm::vec3(0.25, 0, 0));

	matrix = glm::scale(matrix, glm::vec3(1, 0.4, 1)),

	cable->SetModelMatrix(matrix);

	cable->SetModelInnerMatrix(glm::mat4(1.0));

	return cable;
}
