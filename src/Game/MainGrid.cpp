#include "MainGrid.h"

#include "../Utils/loader.h"

MainGrid::MainGrid(
	Video* video,
	CollisionEngine* collisionEngine,
	BaseGrid* baseGrid,
	glm::mat4* extMat)
{
	_video = video;
	_collisionEngine = collisionEngine;
	_baseGrid = baseGrid;
	_extMat = extMat;

	_preview = nullptr;

	int tw;
	int th;
	auto td = Loader::LoadImage("Models/Ship/MainBlocks/Wall.png", tw, th);
	Textures["Wall"] = _video->GetTextures()->AddTexture(tw, th, td);

	auto model = Loader::LoadModel("Models/Ship/MainBlocks/Wall.obj");
	Models["Wall"] = _video->LoadModel(model);

	td = Loader::LoadImage(
		"Models/Ship/MainBlocks/FlightControl.png",
		tw,
		th);
	Textures["FlightControl"] =
		_video->GetTextures()->AddTexture(tw, th, td);

	model = Loader::LoadModel("Models/Ship/MainBlocks/FlightControl.obj");
	Models["FlightControl"] = _video->LoadModel(model);
}

MainGrid::~MainGrid()
{
	for (auto& block : _blocks) {
		_video->RemoveModel(block.second);
		_collisionEngine->RemoveObject(block.second);
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

void MainGrid::InsertBlock(
	int32_t x,
	int32_t y,
	float rotation,
	MainBlock::Type type)
{
	bool available = _baseGrid->GetType(x, y) != BaseBlock::Type::Empty;
	available = available && _blocks.find({x, y}) == _blocks.end();

	if (!available) {
		return;
	}

	MainBlock* block;

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(x, y, 1));

	switch (type) {
	case MainBlock::Type::Wall:
		block = new Wall(x, y, this);
		break;
	case MainBlock::Type::FlightControl:
		block = new FlightControl(x, y, rotation, this);
		break;
	default:
		return;
	}

	if (block->Rotateable()) {
		matrix = glm::rotate(
			matrix,
			glm::radians(rotation),
			glm::vec3(0, 0, 1));
	}

	block->SetDrawEnabled(true);
	block->SetModelMatrix(matrix);
	block->SetObjectMatrix(matrix);
	block->SetModelExternalMatrix(_extMat);
	block->SetObjectExternalMatrix(_extMat);

	block->SetModelInnerMatrix(glm::mat4(1.0));

	_video->RegisterModel(block);
	_collisionEngine->RegisterObject(block);

	_blocks[{x, y}] = block;
}

void MainGrid::RemoveBlock(int32_t x, int32_t y)
{
	if (_blocks.find({x, y}) == _blocks.end()) {
		return;
	}

	_video->RemoveModel(_blocks[{x, y}]);
	_collisionEngine->RemoveObject(_blocks[{x, y}]);

	delete _blocks[{x, y}];
	_blocks.erase({x, y});
}

void MainGrid::PreviewBlock(
	int32_t x,
	int32_t y,
	float rotation,
	MainBlock::Type type)
{
	StopPreview();

	bool available = _baseGrid->GetType(x, y) != BaseBlock::Type::Empty;
	available = available && _blocks.find({x, y}) == _blocks.end();

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(x, y, 1));

	switch (type) {
	case MainBlock::Type::Wall:
		_preview = new Wall(x, y, this);
		break;
	case MainBlock::Type::FlightControl:
		_preview = new FlightControl(x, y, rotation, this);
		break;
	default:
		return;
	}

	if (_preview->Rotateable()) {
		matrix = glm::rotate(
			matrix,
			glm::radians(rotation),
			glm::vec3(0, 0, 1));
	}

	_preview->SetDrawEnabled(true);
	_preview->SetDrawLight(true);
	_preview->SetModelMatrix(matrix);
	_preview->SetModelExternalMatrix(_extMat);

	_preview->SetModelInnerMatrix(glm::scale(
		glm::mat4(1.0),
		glm::vec3(1.01, 1.01, 1.01)));

	if (available) {
		_preview->SetColorMultiplier({0.5, 1, 0.5, 0.4});
	} else {
		_preview->SetColorMultiplier({1.0, 0.5, 0.5, 0.4});
	}

	_video->RegisterModel(_preview);
}

void MainGrid::StopPreview()
{
	if (_preview) {
		_video->RemoveModel(_preview);
		delete _preview;
		_preview = nullptr;
	}
}

MainBlock::Type MainGrid::GetType(int32_t x, int32_t y)
{
	if (_blocks.find({x, y}) == _blocks.end()) {
		return MainBlock::Type::Empty;
	}

	return _blocks[{x, y}]->GetType();
}

MainBlock* MainGrid::GetBlock(int32_t x, int32_t y)
{
	if (_blocks.find({x, y}) == _blocks.end()) {
		return nullptr;
	}

	return _blocks[{x, y}];
}

Wall::Wall(
	int32_t x,
	int32_t y,
	MainGrid* grid) :
	MainBlock(x, y, 0, grid)
{
	SetTexture({_grid->Textures["Wall"]});
	SetModels({_grid->Models["Wall"]});

	auto model = Loader::LoadModel("Models/Ship/MainBlocks/Wall.obj");
	SetObjectVertices(model.Vertices);
	SetObjectIndices(model.Indices);
	SetObjectNormals(model.Normals);
	SetObjectCenter();
	SetObjectDynamic(true);
	SetObjectSphereCenter({0, 0, 0.5});
	SetObjectSphereRadius(0.5);
	SetObjectDomain(1);
}

FlightControl::FlightControl(
	int32_t x,
	int32_t y,
	float rotation,
	MainGrid* grid) :
	MainBlock(x, y, rotation, grid)
{
	SetTexture({_grid->Textures["FlightControl"]});
	SetModels({_grid->Models["FlightControl"]});

	auto model = Loader::LoadModel(
		"Models/Ship/MainBlocks/Wall.obj");

	for (auto& vertex : model.Vertices) {
		vertex.x *= 0.8;
		vertex.y *= 0.8;
		vertex.z *= 1.2 / 3.0;
	}

	SetObjectVertices(model.Vertices);
	SetObjectIndices(model.Indices);
	SetObjectNormals(model.Normals);
	SetObjectCenter();
	SetObjectDynamic(true);
	SetObjectSphereCenter({0, 0, 0.5});
	SetObjectSphereRadius(0.3);
	SetObjectDomain(1);
}
