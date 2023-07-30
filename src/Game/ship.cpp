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
}

Ship::~Ship()
{
	for (auto& block : _grid) {
		delete block.second;
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

	_grid[position] = new Block(_video, position, _blockTexture);
}

void Ship::RemoveBlock(const glm::ivec3& position)
{
	if (_grid.find(position) == _grid.end()) {
		return;
	}

	delete _grid[position];
	_grid.erase(position);
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
