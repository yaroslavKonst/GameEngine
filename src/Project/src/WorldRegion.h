#ifndef _WORLD_REGION
#define _WORLD_REGION

#include "global.h"
#include "GameGlobal.h"

class WorldRegion : public Model
{
public:
	WorldRegion(Engine* engine, int x, int y);
	~WorldRegion();

	void Load(int lod);
	void Unload();

	void SetLod(int lod);

private:
	double _cellSize;
	int _cellCount;

	Engine* _engine;

	int _x;
	int _y;
	bool _loaded;

	int _lod;

	Loader::VertexData* BuildSurface();

	std::map<int, uint32_t> _lodCache;
};

#endif
