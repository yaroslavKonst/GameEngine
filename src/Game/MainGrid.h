#ifndef _MAIN_GRID_H
#define _MAIN_GRID_H

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/CollisionEngine.h"
#include "common.h"
#include "BaseGrid.h"

class MainGrid;

class MainBlock : public Model, public Object
{
public:
	enum class Type
	{
		Empty,
		Wall,
		CommGrate,
		Door,
		Engine,
		Container,
		Cockpit
	};

	MainBlock(int32_t x, int32_t y, float rotation, MainGrid* grid)
	{
		_x = x;
		_y = y;
		_rotation = rotation;
		_grid = grid;
	}

	virtual ~MainBlock()
	{ }

	virtual Type GetType() = 0;

	virtual void Update()
	{ }

	virtual bool CanBePlaced()
	{
		return false;
	}

protected:
	int32_t _x;
	int32_t _y;
	float _rotation;
	MainGrid* _grid;
};

class Wall : public MainBlock
{
public:
	Wall(int32_t x, int32_t y, MainGrid* grid);
	~Wall()
	{ }

	Type GetType() override
	{
		return Type::Wall;
	}
};

class MainGrid
{
public:
	Library Textures;
	Library Models;

	MainGrid(
		Video* video,
		CollisionEngine* collisionEngine,
		BaseGrid* baseGrid);
	~MainGrid();

	void InsertBlock(
		int32_t x,
		int32_t y,
		float rotation,
		MainBlock::Type type);
	void RemoveBlock(int32_t x, int32_t y);

	void PreviewBlock(
		int32_t x,
		int32_t y,
		float rotation,
		MainBlock::Type type);
	void StopPreview();

	MainBlock::Type GetType(int32_t x, int32_t y);
	MainBlock* GetBlock(int32_t x, int32_t y);

private:
	std::map<Coord2D, MainBlock*> _blocks;

	Video* _video;
	CollisionEngine* _collisionEngine;
	BaseGrid* _baseGrid;

	MainBlock* _preview;
};

#endif
