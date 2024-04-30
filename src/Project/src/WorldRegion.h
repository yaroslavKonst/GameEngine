#ifndef _WORLD_REGION
#define _WORLD_REGION

#include "global.h"
#include "GameGlobal.h"

class WorldRegion : public Model
{
public:
	static constexpr double CellSize = 0.5;
	static constexpr int CellCount = 100;

	WorldRegion(Engine* engine, int x, int y);
	~WorldRegion();

	void Load();
	void Unload();

private:
	Engine* _engine;

	int _x;
	int _y;
	bool _loaded;

	void BuildSurface();
};

#endif
