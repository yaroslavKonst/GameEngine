#ifndef _BASE_GRID_H
#define _BASE_GRID_H

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/CollisionEngine.h"
#include "common.h"

class BaseGrid;

class BaseBlock: public Model, public Object
{
public:
	enum class Type
	{
		Empty,
		Floor,
		FloorComm
	};

	BaseBlock(int32_t x, int32_t y, BaseGrid* grid)
	{
		_x = x;
		_y = y;
		_grid = grid;
	}

	virtual ~BaseBlock()
	{ }

	virtual Type GetType() = 0;

	virtual void Update()
	{ }

protected:
	int32_t _x;
	int32_t _y;
	BaseGrid* _grid;
};

class FloorBlock: public BaseBlock
{
public:
	FloorBlock(int32_t x, int32_t y, BaseGrid* grid);
	~FloorBlock()
	{ }

	Type GetType() override
	{
		return Type::Floor;
	};

private:
};

class FloorCommBlock: public BaseBlock
{
public:
	FloorCommBlock(
		int32_t x,
		int32_t y,
		BaseGrid* grid,
		Video* video,
		glm::mat4* extMat,
		bool preview = false);
	~FloorCommBlock();

	Type GetType() override
	{
		return Type::FloorComm;
	};

	virtual void Update() override;

	virtual uint32_t RayCastCallback(void* userPointer) override
	{
		return 1;
	}

	bool GetPowerCable();
	void SetPowerCable(bool value);

	bool GetDataCable();
	void SetDataCable(bool value);

private:
	Video* _video;
	std::vector<Model*> _delims;
	Model* _powerCableHub;
	Model* _dataCableHub;
	std::vector<Model*> _powerCables;
	std::vector<Model*> _dataCables;

	bool _hasPowerCable;
	bool _hasDataCable;

	Model* CreateDelim(size_t i);
	Model* CreatePowerCable(size_t i);
	Model* CreateDataCable(size_t i);

	glm::mat4* _extMat;
};

class BaseGrid
{
public:
	Library Textures;
	Library Models;

	BaseGrid(
		Video* video,
		CollisionEngine* collisionEngine,
		glm::mat4* extMat);
	~BaseGrid();

	void InsertBlock(
		int32_t x,
		int32_t y,
		BaseBlock::Type type,
		bool force = false);
	void RemoveBlock(int32_t x, int32_t y);

	void PreviewBlock(int32_t x, int32_t y, BaseBlock::Type type);
	void StopPreview();

	BaseBlock::Type GetType(int32_t x, int32_t y);
	BaseBlock* GetBlock(int32_t x, int32_t y);

private:
	std::map<Coord2D, BaseBlock*> _blocks;

	BaseBlock* _preview;

	Video* _video;
	CollisionEngine* _collisionEngine;
	glm::mat4* _extMat;
};

#endif
