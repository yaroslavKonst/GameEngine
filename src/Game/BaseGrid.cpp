#include "BaseGrid.h"

#include "../Utils/loader.h"

#include "../Logger/logger.h"

BaseGrid::BaseGrid(
	Video* video,
	CollisionEngine* collisionEngine,
	glm::mat4* extMat)
{
	_video = video;
	_collisionEngine = collisionEngine;
	_extMat = extMat;

	auto td = Loader::LoadImage("Models/Ship/ShipFloor.png");
	Textures["Floor"] = _video->AddTexture(td);

	td = Loader::LoadImage("Models/Ship/ShipFloorComm.png");
	Textures["FloorComm"] = _video->AddTexture(td);

	td = Loader::LoadImage("Models/Ship/ShipFloorCommDelim.png");
	Textures["FloorCommDelim"] = _video->AddTexture(td);

	td = Loader::LoadImage("Models/Ship/ShipPowerCableHub.png");
	Textures["PowerCableHub"] = _video->AddTexture(td);

	td = Loader::LoadImage("Models/Ship/ShipPowerCable.png");
	Textures["PowerCable"] = _video->AddTexture(td);

	td = Loader::LoadImage("Models/Ship/ShipDataCable.png");
	Textures["DataCable"] = _video->AddTexture(td);

	td = Loader::LoadImage("Models/Ship/ShipDataCableHub.png");
	Textures["DataCableHub"] = _video->AddTexture(td);

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
		_video->RemoveTexture(texture.second);
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
			_video,
			_extMat);
		break;
	default:
		return;
	}

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(x, y, 0));

	block->DrawParams.Enabled = true;
	block->ModelParams.Matrix = matrix;
	block->ModelParams.ExternalMatrix = _extMat;
	block->SetObjectExternalMatrix(_extMat);

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
			_video,
			_extMat,
			true);
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

	_preview->DrawParams.Enabled = true;
	_preview->TextureParams.IsLight = true;

	if (available) {
		_preview->DrawParams.ColorMultiplier = {0.5, 1, 0.5, 0.4};
	} else {
		_preview->DrawParams.ColorMultiplier = {1.0, 0.5, 0.5, 0.4};
	}

	_preview->ModelParams.Matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(x, y, 0));

	_preview->ModelParams.ExternalMatrix = _extMat;

	_preview->ModelParams.InnerMatrix = glm::scale(
		glm::mat4(1.0),
		glm::vec3(1.01, 1.01, 1.01));

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

glm::vec3 BaseGrid::GetCollisionFeedback()
{
	glm::vec3 maxForce(0, 0, 0);

	for (auto& block : _blocks) {
		glm::vec3 effect = block.second->GetObjectEffect();

		if (glm::length(effect) > glm::length(maxForce)) {
			maxForce = effect;
		}
	}

	return maxForce;
}

FloorBlock::FloorBlock(
	int32_t x,
	int32_t y,
	BaseGrid* grid) :
	BaseBlock(x, y, grid)
{
	TextureParams.SetAll(grid->Textures["Floor"]);
	ModelParams.Model = grid->Models["Floor"];
}

FloorCommBlock::FloorCommBlock(
	int32_t x,
	int32_t y,
	BaseGrid* grid,
	Video* video,
	glm::mat4* extMat,
	bool preview) :
	BaseBlock(x, y, grid)
{
	_video = video;
	TextureParams.SetAll(grid->Textures["FloorComm"]);
	ModelParams.Model = grid->Models["FloorComm"];
	_delims.resize(4, nullptr);
	_powerCables.resize(4, nullptr);
	_dataCables.resize(4, nullptr);
	_powerCableHub = nullptr;
	_dataCableHub = nullptr;
	_extMat = extMat;

	_hasPowerCable = false;
	_hasDataCable = false;

	if (preview) {
		return;
	}

	for (size_t i = 0; i < 4; ++i) {
		_delims[i] = CreateDelim(i);
		_delims[i]->DrawParams.Enabled = true;
		_delims[i]->TextureParams.SetAll(
			grid->Textures["FloorCommDelim"]);
		_delims[i]->ModelParams.Model = grid->Models["FloorCommDelim"];
		_delims[i]->ModelParams.ExternalMatrix = extMat;

		_powerCables[i] = CreatePowerCable(i);
		_powerCables[i]->DrawParams.Enabled = false;
		_powerCables[i]->TextureParams.SetAll(
			grid->Textures["PowerCable"]);
		_powerCables[i]->ModelParams.Model = grid->Models["PowerCable"];
		_powerCables[i]->ModelParams.ExternalMatrix = extMat;

		_dataCables[i] = CreateDataCable(i);
		_dataCables[i]->DrawParams.Enabled = false;
		_dataCables[i]->TextureParams.SetAll(
			grid->Textures["DataCable"]);
		_dataCables[i]->ModelParams.Model = grid->Models["DataCable"];
		_dataCables[i]->ModelParams.ExternalMatrix = extMat;

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

	if (_dataCableHub) {
		_video->RemoveModel(_dataCableHub);
		delete _dataCableHub;
		_dataCableHub = nullptr;
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
			_delims[i]->DrawParams.Enabled = true;
			_powerCables[i]->DrawParams.Enabled = false;
			_dataCables[i]->DrawParams.Enabled = false;
		} else {
			power = static_cast<FloorCommBlock*>(
				_grid->GetBlock(x[i], y[i]))->GetPowerCable();
			data = static_cast<FloorCommBlock*>(
				_grid->GetBlock(x[i], y[i]))->GetDataCable();

			power = power && _hasPowerCable;
			data = data && _hasDataCable;

			_delims[i]->DrawParams.Enabled = !(power || data);
			_powerCables[i]->DrawParams.Enabled = power;
			_dataCables[i]->DrawParams.Enabled = data;
		}
	}

	if (_hasPowerCable) {
		if (
			_powerCables[0]->DrawParams.Enabled &&
			_powerCables[1]->DrawParams.Enabled &&
			!_powerCables[2]->DrawParams.Enabled &&
			!_powerCables[3]->DrawParams.Enabled)
		{
			_powerCableHub->DrawParams.Enabled = false;
		} else if (
			!_powerCables[0]->DrawParams.Enabled &&
			!_powerCables[1]->DrawParams.Enabled &&
			_powerCables[2]->DrawParams.Enabled &&
			_powerCables[3]->DrawParams.Enabled)
		{
			_powerCableHub->DrawParams.Enabled = false;
		} else {
			_powerCableHub->DrawParams.Enabled = true;
		}
	}

	if (_hasDataCable) {
		if (
			_dataCables[0]->DrawParams.Enabled &&
			_dataCables[1]->DrawParams.Enabled &&
			!_dataCables[2]->DrawParams.Enabled &&
			!_dataCables[3]->DrawParams.Enabled)
		{
			_dataCableHub->DrawParams.Enabled = false;
		} else if (
			!_dataCables[0]->DrawParams.Enabled &&
			!_dataCables[1]->DrawParams.Enabled &&
			_dataCables[2]->DrawParams.Enabled &&
			_dataCables[3]->DrawParams.Enabled)
		{
			_dataCableHub->DrawParams.Enabled = false;
		} else {
			_dataCableHub->DrawParams.Enabled = true;
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

		_powerCableHub->ModelParams.Model =
			_grid->Models["PowerCableHub"];
		_powerCableHub->TextureParams.SetAll(
			_grid->Textures["PowerCableHub"]);
		_powerCableHub->DrawParams.ColorMultiplier = {1, 0.8, 0.8, 1};

		glm::mat4 matrix = ModelParams.Matrix;

		matrix = glm::translate(
			matrix,
			glm::vec3(0, 0, 0.8));

		_powerCableHub->DrawParams.Enabled = true;
		_powerCableHub->ModelParams.Matrix = matrix;
		_powerCableHub->ModelParams.ExternalMatrix = _extMat;

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

		_dataCableHub->ModelParams.Model =
			_grid->Models["DataCableHub"];
		_dataCableHub->TextureParams.SetAll(
			_grid->Textures["DataCableHub"]);
		_dataCableHub->DrawParams.ColorMultiplier = {0.8, 0.8, 1, 1};

		glm::mat4 matrix = ModelParams.Matrix;

		matrix = glm::translate(
			matrix,
			glm::vec3(0, 0, 0.8));

		_dataCableHub->DrawParams.Enabled = true;
		_dataCableHub->ModelParams.Matrix = matrix;
		_dataCableHub->ModelParams.ExternalMatrix = _extMat;

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

	delim->ModelParams.Matrix = matrix;

	return delim;
}

Model* FloorCommBlock::CreatePowerCable(size_t i)
{
	Model* cable = new Model;

	cable->DrawParams.ColorMultiplier = {1, 0.8, 0.8, 1};

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

	cable->ModelParams.Matrix = matrix;

	return cable;
}

Model* FloorCommBlock::CreateDataCable(size_t i)
{
	Model* cable = new Model;

	cable->DrawParams.ColorMultiplier = {0.8, 0.8, 1, 1};

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

	cable->ModelParams.Matrix = matrix;

	return cable;
}
