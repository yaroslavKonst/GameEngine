#ifndef _SHIP_H
#define _SHIP_H

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/CollisionEngine.h"
#include "../UniverseEngine/actor.h"

class BaseGrid;

typedef std::map<std::string, uint32_t> Library;

struct Coord2D
{
	int32_t X;
	int32_t Y;

	bool operator<(const Coord2D& coord) const
	{
		if (X != coord.X) {
			return X < coord.X;
		}

		return Y < coord.Y;
	}
};

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
};

class BaseGrid
{
public:
	Library Textures;
	Library Models;

	BaseGrid(Video* video, CollisionEngine* collisionEngine);
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
};

class Ship : public InputHandler, public Actor
{
public:
	Ship(Video* video, CollisionEngine* collisionEngine);
	~Ship();

	void Tick() override;

	void Key(int key, int scancode, int action, int mods) override;

private:
	Video* _video;
	CollisionEngine* _collisionEngine;

	BaseGrid* _baseGrid;

	int32_t _buildX;
	int32_t _buildY;
	BaseBlock::Type _buildType;
};

#endif
