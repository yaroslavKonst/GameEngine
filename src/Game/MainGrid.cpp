#include "MainGrid.h"

#include "../Utils/loader.h"
#include "../PhysicalEngine/PlaneHelper.h"
#include "../Logger/logger.h"

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

	auto td = Loader::LoadImage("Models/Ship/MainBlocks/Wall.png");
	Textures["Wall"] = _video->GetTextures()->AddTexture(td);

	auto model = Loader::LoadModel("Models/Ship/MainBlocks/Wall.obj");
	Models["Wall"] = _video->LoadModel(model);

	td = Loader::LoadImage(
		"Models/Ship/MainBlocks/FlightControl.png");
	Textures["FlightControl"] =
		_video->GetTextures()->AddTexture(td);

	model = Loader::LoadModel("Models/Ship/MainBlocks/FlightControl.obj");
	Models["FlightControl"] = _video->LoadModel(model);

	td = Loader::LoadImage(
		"Models/Ship/MainBlocks/StaticThruster.png");
	Textures["StaticThruster"] =
		_video->GetTextures()->AddTexture(td);

	model = Loader::LoadModel("Models/Ship/MainBlocks/StaticThruster.obj");
	Models["StaticThruster"] = _video->LoadModel(model);

	td = Loader::LoadImage(
		"Models/Ship/MainBlocks/DynamicThruster.png");
	Textures["DynamicThruster"] =
		_video->GetTextures()->AddTexture(td);

	model = Loader::LoadModel("Models/Ship/MainBlocks/DynamicThruster.obj");
	Models["DynamicThruster"] = _video->LoadModel(model);

	td = Loader::LoadImage(
		"Models/Ship/MainBlocks/DynamicThrusterBase.png");
	Textures["DynamicThrusterBase"] =
		_video->GetTextures()->AddTexture(td);

	model = Loader::LoadModel(
		"Models/Ship/MainBlocks/DynamicThrusterBase.obj");
	Models["DynamicThrusterBase"] = _video->LoadModel(model);
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
	case MainBlock::Type::StaticThruster:
		block = new StaticThruster(x, y, rotation, this);
		_staticThrusters.insert(static_cast<StaticThruster*>(block));
		break;
	case MainBlock::Type::DynamicThruster:
		block = new DynamicThruster(
			x,
			y,
			rotation,
			this,
			_video,
			_extMat);
		_dynamicThrusters.insert(static_cast<DynamicThruster*>(block));
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

	block->DrawParams.Enabled = true;
	block->ModelParams.Matrix = matrix;
	block->SetObjectMatrix(matrix);
	block->ModelParams.ExternalMatrix = _extMat;
	block->SetObjectExternalMatrix(_extMat);

	_video->RegisterModel(block);
	_collisionEngine->RegisterObject(block);

	_blocks[{x, y}] = block;
}

void MainGrid::RemoveBlock(int32_t x, int32_t y)
{
	if (_blocks.find({x, y}) == _blocks.end()) {
		return;
	}

	MainBlock* block = _blocks[{x, y}];

	_video->RemoveModel(block);
	_collisionEngine->RemoveObject(block);

	if (block->GetType() == MainBlock::Type::StaticThruster) {
		_staticThrusters.erase(static_cast<StaticThruster*>(block));
	}

	if (block->GetType() == MainBlock::Type::DynamicThruster) {
		_dynamicThrusters.erase(static_cast<DynamicThruster*>(block));
	}

	delete block;
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
	case MainBlock::Type::StaticThruster:
		_preview = new StaticThruster(x, y, rotation, this);
		break;
	case MainBlock::Type::DynamicThruster:
		_preview = new DynamicThruster(
			x,
			y,
			rotation,
			this,
			_video,
			_extMat,
			available ?
				glm::vec4(0.5, 1, 0.5, 0.4) :
				glm::vec4(1.0, 0.5, 0.5, 0.4));
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

	_preview->DrawParams.Enabled = true;
	_preview->DrawParams.IsLight = true;
	_preview->ModelParams.Matrix = matrix;
	_preview->ModelParams.ExternalMatrix = _extMat;

	_preview->ModelParams.InnerMatrix = glm::scale(
		glm::mat4(1.0),
		glm::vec3(1.01, 1.01, 1.01));

	if (available) {
		_preview->DrawParams.ColorMultiplier = {0.5, 1, 0.5, 0.4};
	} else {
		_preview->DrawParams.ColorMultiplier = {1.0, 0.5, 0.5, 0.4};
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

glm::vec3 MainGrid::SetThrusterForce(const glm::vec3& force)
{
	glm::vec3 resForce(0.0);

	for (auto& thruster : _dynamicThrusters) {
		resForce += thruster->SetDirection(force);
	}

	for (auto& thruster : _staticThrusters) {
		resForce += thruster->SetDirection(force);
	}

	float coeff = glm::length(resForce) / glm::length(force);

	if (coeff > 1) {
		resForce /= coeff;
	}

	return resForce;
}

Wall::Wall(
	int32_t x,
	int32_t y,
	MainGrid* grid) :
	MainBlock(x, y, 0, grid)
{
	TextureParams.SetAll(_grid->Textures["Wall"]);
	ModelParams.Model = _grid->Models["Wall"];

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
	TextureParams.SetAll(_grid->Textures["FlightControl"]);
	ModelParams.Model = _grid->Models["FlightControl"];

	auto model = Loader::LoadModel(
		"Models/Ship/MainBlocks/Wall.obj");

	for (auto& vertex : model.Vertices) {
		vertex.x *= 0.8;
		vertex.y *= 0.5;
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

StaticThruster::StaticThruster(
	int32_t x,
	int32_t y,
	float rotation,
	MainGrid* grid) :
	MainBlock(x, y, rotation, grid)
{
	TextureParams.SetAll(_grid->Textures["StaticThruster"]);
	ModelParams.Model = _grid->Models["StaticThruster"];

	auto model = Loader::LoadModel(
		"Models/Ship/MainBlocks/StaticThrusterCollision.obj");
	SetObjectVertices(model.Vertices);
	SetObjectIndices(model.Indices);
	SetObjectNormals(model.Normals);
	SetObjectCenter();
	SetObjectDynamic(true);
	SetObjectSphereCenter({0, 0, 0.5});
	SetObjectSphereRadius(0.5);
	SetObjectDomain(1);
}

glm::vec3 StaticThruster::SetDirection(const glm::vec3& value)
{
	glm::vec3 thrDir(0, 1, 0);

	thrDir = *ModelParams.ExternalMatrix * ModelParams.Matrix *
		glm::vec4(thrDir, 0.0);

	float angleCos =
		glm::dot(glm::normalize(thrDir), glm::normalize(value));

	if (angleCos > 0) {
		return glm::normalize(thrDir) * angleCos;
	}

	return glm::vec3(0, 0, 0);
}

DynamicThruster::DynamicThruster(
	int32_t x,
	int32_t y,
	float rotation,
	MainGrid* grid,
	Video* video,
	glm::mat4* extMat,
	glm::vec4 colorMultiplier) :
	MainBlock(x, y, rotation, grid)
{
	_video = video;
	_extMat = extMat;

	TextureParams.SetAll(_grid->Textures["DynamicThrusterBase"]);
	ModelParams.Model = _grid->Models["DynamicThrusterBase"];

	auto model = Loader::LoadModel(
		"Models/Ship/MainBlocks/DynamicThrusterBase.obj");
	SetObjectVertices(model.Vertices);
	SetObjectIndices(model.Indices);
	SetObjectNormals(model.Normals);
	SetObjectCenter();
	SetObjectDynamic(true);
	SetObjectSphereCenter({0, 0, 0.5});
	SetObjectSphereRadius(0.5);
	SetObjectDomain(1);

	_angle = 0;

	_thruster = new Model;

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(x, y, 1));

	matrix = glm::rotate(
		matrix,
		glm::radians(rotation),
		glm::vec3(0, 0, 1));

	matrix = glm::translate(
		matrix,
		glm::vec3(0.0, -0.5, 1.5));

	matrix = glm::rotate(
		matrix,
		glm::radians(_angle),
		glm::vec3(0, 1, 0));

	_thruster->TextureParams.SetAll(_grid->Textures["DynamicThruster"]);
	_thruster->ModelParams.Model = _grid->Models["DynamicThruster"];

	_thruster->DrawParams.Enabled = true;
	_thruster->ModelParams.Matrix = matrix;
	_thruster->ModelParams.ExternalMatrix = _extMat;

	_thruster->DrawParams.ColorMultiplier = colorMultiplier;

	if (colorMultiplier.a < 1) {
		_thruster->DrawParams.IsLight = true;
	}

	_video->RegisterModel(_thruster);
}

DynamicThruster::~DynamicThruster()
{
	_video->RemoveModel(_thruster);
	delete _thruster;
}

glm::vec3 DynamicThruster::SetDirection(const glm::vec3& value)
{
	glm::vec3 p1(0, 0, 0);
	glm::vec3 p2(1, 0, 0);
	glm::vec3 pc(0, 0, -1);
	glm::vec3 pn(0, 1, 0);

	glm::vec3 aDir(0, cosf(glm::radians(20.0)), 1);

	glm::mat4 matrix = *ModelParams.ExternalMatrix * ModelParams.Matrix;
	glm::mat4 rotMatrix = *ModelParams.ExternalMatrix *
		_thruster->ModelParams.Matrix;

	p1 = matrix * glm::vec4(p1, 1.0f);
	p2 = matrix * glm::vec4(p2, 1.0f);
	glm::vec3 currentDir = rotMatrix * glm::vec4(pc, 0.0f);
	aDir = rotMatrix * glm::vec4(aDir, 0.0f);
	pc = matrix * glm::vec4(pc, 1.0f);
	pn = matrix * glm::vec4(pn, 1.0f);

	PlaneHelper::Plane enginePlane =
		PlaneHelper::PlaneByThreePoints(p1, p2, pc);

	enginePlane[3] = 0;

	glm::vec3 normal = pn - p1;
	glm::vec3 valueDir =
		PlaneHelper::ProjectPointToPlane(value, enginePlane);

	valueDir = glm::normalize(valueDir);
	currentDir = glm::normalize(currentDir);

	glm::vec3 spinDir = glm::cross(valueDir, currentDir);
	float dot = glm::dot(valueDir, currentDir);

	float crossDot = glm::dot(spinDir, glm::normalize(normal));

	if (crossDot > 0) {
		if (dot > 0 || fabs(crossDot) > 0.2) {
			_angle += 1;
		} else {
			_angle += 0.1;
		}
	} else if (crossDot < 0) {
		if (dot > 0 || fabs(crossDot) > 0.2) {
			_angle -= 1;
		} else {
			_angle -= 0.1;
		}
	}

	if (_angle < 0) {
		_angle += 360;
	} else if (_angle >= 360) {
		_angle -= 360;
	}

	glm::mat4 thrMatrix = glm::translate(
		glm::mat4(1.0),
		glm::vec3(_x, _y, 1));

	thrMatrix = glm::rotate(
		thrMatrix,
		glm::radians(_rotation),
		glm::vec3(0, 0, 1));

	thrMatrix = glm::translate(
		thrMatrix,
		glm::vec3(0.0, -0.5, 1.5));

	thrMatrix = glm::rotate(
		thrMatrix,
		glm::radians(_angle),
		glm::vec3(0, 1, 0));

	_thruster->ModelParams.Matrix = thrMatrix;

	float angleCos = glm::dot(glm::normalize(aDir), glm::normalize(value));

	if (angleCos > 0) {
		return glm::normalize(aDir) * angleCos;
	}

	return glm::vec3(0, 0, 0);
}
