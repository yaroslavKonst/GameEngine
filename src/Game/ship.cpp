#include "ship.h"

#include "../Utils/loader.h"

// Ship
Ship::Ship(
	Video* video,
	uint32_t blockTexture,
	CollisionEngine* collisionEngine)
{
	_video = video;
	_collisionEngine = collisionEngine;

	_pos = {0, 0, 1.5};
	_rotation = {0, 0, 0};
	_linearSpeed = {0, 0, 0};
	_angularSpeed = {0, 0, 0};

	_globalMatrix = glm::translate(
		glm::mat4(1),
		glm::vec3(_pos));

	_blockTexture = blockTexture;
	_previewBlock = nullptr;
	_block = nullptr;
	_block = new Block(_video, {0, 0, 0}, {0, 0, 0}, _blockTexture);
}

Ship::~Ship()
{
	if (_block) {
		delete _block;
	}

	if (_previewBlock) {
		delete _previewBlock;
	}

	for (auto& block : _grid) {
		_collisionEngine->RemoveObject(&block.second->Collision);
		delete block.second;
	}
}

void Ship::Tick()
{
	_pos += _linearSpeed;

	_globalMatrix = glm::translate(
		glm::mat4(1),
		glm::vec3(_pos));

	_block->SetModelMatrix(_globalMatrix);
}

void Ship::InsertBlock(
	const glm::ivec3& position,
	const glm::vec3& rotation)
{
	if (_grid.find(position) != _grid.end()) {
		return;
	}

	_block->SetDrawEnabled(true);

	glm::mat4 matrix = glm::translate(
		glm::mat4(1),
		glm::vec3(position) * 2.0f);

	matrix = glm::rotate(
		matrix,
		glm::radians(rotation[0]),
		glm::vec3(0,0,1));
	matrix = glm::rotate(
		matrix,
		glm::radians(rotation[1]),
		glm::vec3(1,0,0));
	matrix = glm::rotate(
		matrix,
		glm::radians(rotation[2]),
		glm::vec3(0,1,0));

	BlockDescriptor* desc = new BlockDescriptor();
	desc->Matrix = matrix;
	desc->Collision.SetObjectVertices(_block->GetModelVertices());
	desc->Collision.SetObjectIndices(_block->GetModelIndices());
	desc->Collision.SetObjectCenter();
	desc->Collision.SetObjectMatrix(matrix);
	desc->Collision.SetObjectExternMatrix(&_globalMatrix);
	desc->Collision.SetObjectDomain(1);
	desc->Collision.SetObjectDynamic(true);

	_collisionEngine->RegisterObject(&desc->Collision);

	_grid[position] = desc;

	UpdateView();
}

void Ship::RemoveBlock(const glm::ivec3& position)
{
	if (_grid.find(position) == _grid.end()) {
		return;
	}

	_collisionEngine->RemoveObject(&_grid[position]->Collision);
	delete _grid[position];

	_grid.erase(position);
	UpdateView();
}

void Ship::PreviewBlock(
	const glm::ivec3& position,
	const glm::vec3& rotation)
{
	StopPreview();
	_previewBlock = new Block(_video, position, rotation, _blockTexture);
	_previewBlock->SetDrawEnabled(true);
	_previewBlock->SetModelMatrix(
		_globalMatrix * _previewBlock->GetModelMatrix());
	_previewBlock->SetModelInnerMatrix(glm::scale(
		glm::mat4(1),
		glm::vec3(1.1, 1.1, 1.1)));
}

void Ship::StopPreview()
{
	if (!_previewBlock) {
		return;
	}

	delete _previewBlock;
	_previewBlock = nullptr;
}

void Ship::UpdateView()
{
	std::vector<glm::mat4> instances(_grid.size());

	size_t idx = 0;

	for (auto& mat : _grid) {
		instances[idx] = mat.second->Matrix;
		++idx;
	}

	_block->SetModelInstances(instances);
}

// Block
Block::Block(
	Video* video,
	const glm::ivec3& position,
	const glm::vec3& rotation,
	uint32_t texture)
{
	_video = video;

	auto model = Loader::LoadModel(
		"../src/Assets/Resources/Models/ship_wall.obj");

	SetModelVertices(model.Vertices);
	SetModelIndices(model.Indices);
	SetModelNormals(model.Normals);
	SetModelTexCoords(model.TexCoords);

	SetModelInnerMatrix(glm::mat4(1));
	SetModelInstances({glm::mat4(1)});

	glm::mat4 matrix = glm::translate(
		glm::mat4(1),
		glm::vec3(position) * 2.0f);

	matrix = glm::rotate(
		matrix,
		glm::radians(rotation[0]),
		glm::vec3(0,0,1));
	matrix = glm::rotate(
		matrix,
		glm::radians(rotation[1]),
		glm::vec3(1,0,0));
	matrix = glm::rotate(
		matrix,
		glm::radians(rotation[2]),
		glm::vec3(0,1,0));

	SetModelMatrix(matrix);

	SetTexture({texture});

	_video->RegisterModel(this);
}

Block::~Block()
{
	_video->RemoveModel(this);
}
