#ifndef _SHIP_H
#define _SHIP_H

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/CollisionEngine.h"
#include "../UniverseEngine/actor.h"

class BaseGrid;

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

	BaseBlock()
	{ }

	virtual ~BaseBlock()
	{ }

	virtual Type GetType() = 0;

	virtual void Update(BaseGrid* grid, int32_t x, int32_t y)
	{ }

private:
};

class FloorBlock: public BaseBlock
{
public:
	FloorBlock(uint32_t texture);
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
		uint32_t texture,
		Video* video,
		uint32_t delimTexture,
		uint32_t powerCableHubTexture);
	~FloorCommBlock();

	Type GetType() override
	{
		return Type::FloorComm;
	};

	virtual void Update(BaseGrid* grid, int32_t x, int32_t y) override;

	virtual uint32_t RayCastCallback(void* userPointer) override
	{
		return 1;
	}

	bool GetPowerCable();
	void SetPowerCable(bool value);

private:
	Video* _video;
	std::vector<Model*> _delims;
	Model* _powerCableHub;
	std::vector<Model*> _powerCables;

	uint32_t _delimTexture;
	uint32_t _powerCableTexture;
	uint32_t _powerCableHubTexture;

	bool _hasPowerCable;
};

class BaseGrid
{
public:
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

	uint32_t _floorTexture;
	uint32_t _floorCommTexture;
	uint32_t _floorCommDelimTexture;
	uint32_t _powerCableHubTexture;

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
