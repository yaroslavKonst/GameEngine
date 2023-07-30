#include "ship.h"

#include "../Utils/loader.h"

// Ship
Ship::Ship(Video* video, uint32_t blockTexture)
{
	_video = video;

	_pos = {0, 0, 0};
	_rotation = {0, 0, 0};
	_linearSpeed = {0, 0, 0};
	_angularSpeed = {0, 0, 0};

	_blockTexture = blockTexture;
	_previewBlock = nullptr;
	_block = nullptr;
}

Ship::~Ship()
{
	if (_block) {
		delete _block;
	}

	if (_previewBlock) {
		delete _previewBlock;
	}
}

void Ship::Tick()
{
}

void Ship::InsertBlock(const glm::ivec3& position)
{
	if (_grid.find(position) != _grid.end()) {
		return;
	}

	glm::mat4 matrix = glm::translate(
		glm::mat4(1),
		glm::vec3(position) * 2.0f);

	_grid[position] = matrix;

	UpdateView();
}

void Ship::RemoveBlock(const glm::ivec3& position)
{
	if (_grid.find(position) == _grid.end()) {
		return;
	}

	_grid.erase(position);
	UpdateView();
}

void Ship::PreviewBlock(const glm::ivec3& position)
{
	StopPreview();
	_previewBlock = new Block(_video, position, _blockTexture);
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
		instances[idx] = mat.second;
		++idx;
	}

	if (!_block) {
		_block = new Block(_video, {0, 0, 0}, _blockTexture);
	}

	_block->SetModelInstances(instances);
}

// Block
Block::Block(Video* video, const glm::ivec3& position, uint32_t texture)
{
	_video = video;

	auto model = Loader::LoadModel(
		"../src/Assets/Resources/Models/blade.obj");

	SetModelVertices(model.Vertices);
	SetModelIndices(model.Indices);
	SetModelNormals(model.Normals);
	SetModelTexCoords(model.TexCoords);

	SetObjectVertices(model.Vertices);
	SetObjectIndices(model.Indices);

	SetModelInnerMatrix(glm::mat4(1));
	SetModelInstances({glm::mat4(1)});

	SetModelMatrix(
		glm::translate(glm::mat4(1),
			glm::vec3(position) * 2.0f));

	SetTexture({texture});

	SetDrawEnabled(true);

	_video->RegisterModel(this);
}

Block::~Block()
{
	_video->RemoveModel(this);
}
