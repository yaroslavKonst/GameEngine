#include "WorldRegion.h"

#include "../../Engine/Math/transform.h"
#include "surface.h"

WorldRegion::WorldRegion(Engine* engine, int x, int y)
{
	_engine = engine;
	_x = x;
	_y = y;

	_loaded = false;
}

WorldRegion::~WorldRegion()
{
	Unload();
}

static int Pow2(int e)
{
	int res = 1;

	while (e > 0) {
		res *= 2;
		--e;
	}

	return res;
}

void WorldRegion::Load(int lod)
{
	if (_loaded && (_lod == lod || _lod == lod / 2)) {
		return;
	}

	_lod = lod;

	lod = Pow2(_lod);

	_cellCount = 128 / lod;
	_cellSize = 0.5 * lod;

	if (_loaded) {
		Unload();
	}

	auto textureData = Loader::LoadImage("World/Textures/Grass.png");
	TextureParams.SetAll(_engine->video->LoadTexture(textureData));

	BuildSurface();

	ModelParams.Matrix = Math::Translate({
		(_x * _cellCount - (double)_cellCount / 2.0) * _cellSize,
		(_y * _cellCount - (double)_cellCount / 2.0) * _cellSize,
		0.0
	});

	_engine->video->RegisterModel(this);
	DrawParams.Enabled = true;

	_loaded = true;
}

void WorldRegion::Unload()
{
	if (!_loaded) {
		return;
	}

	_engine->video->RemoveModel(this);
	_engine->video->UnloadModel(ModelParams.Model);
	_engine->video->UnloadTexture(TextureParams.Diffuse);

	_loaded = false;
}

void WorldRegion::BuildSurface()
{
	Loader::VertexData geometry;

	geometry.Instances = {Math::Mat<4>(1.0)};

	for (int vy = 0; vy <= _cellCount; ++vy) {
		for (int vx = 0; vx <= _cellCount; ++vx) {
			Math::Vec<3> vertex = {
				(double)vx * _cellSize,
				(double)vy * _cellSize,
				0.0
			};

			double x = vertex[0] +
				(_x * _cellCount - (double)_cellCount / 2.0) *
				_cellSize;
			double y = vertex[1] +
				(_y * _cellCount - (double)_cellCount / 2.0) *
				_cellSize;

			vertex[2] = Surface::Height(x, y);

			double delta = _cellSize / 10.0;

			double h = vertex[2];
			double hdx = Surface::Height(x + delta, y);
			double hdy = Surface::Height(x, y + delta);

			Math::Vec<3> vecX = {
				delta,
				0,
				hdx - h
			};

			Math::Vec<3> vecY = {
				0,
				delta,
				hdy - h
			};

			Math::Vec<3> normal = vecX.Cross(vecY).Normalize();

			Math::Vec<2> texCoord = {
				(double)vx * _cellSize,
				(double)vy * _cellSize
			};

			Loader::VertexData::MatrixIndex matIdx;
			matIdx.Index[0] = 0;
			matIdx.Index[1] = 0;
			matIdx.Coeff[0] = 1.0;
			matIdx.Coeff[1] = 0.0;

			geometry.Vertices.push_back(vertex);
			geometry.Normals.push_back(normal);
			geometry.TexCoords.push_back(texCoord);
			geometry.MatrixIndices.push_back(matIdx);
		}
	}

	for (int vy = 0; vy < _cellCount; ++vy) {
		for (int vx = 0; vx < _cellCount; ++vx) {
			uint32_t i0 = vy * (_cellCount + 1) + vx;
			uint32_t i1 = vy * (_cellCount + 1) + vx + 1;
			uint32_t i2 = (vy + 1) * (_cellCount + 1) + vx;

			geometry.Indices.push_back(i0);
			geometry.Indices.push_back(i1);
			geometry.Indices.push_back(i2);

			i0 = (vy + 1) * (_cellCount + 1) + vx;
			i1 = vy * (_cellCount + 1) + vx + 1;
			i2 = (vy + 1) * (_cellCount + 1) + vx + 1;

			geometry.Indices.push_back(i0);
			geometry.Indices.push_back(i1);
			geometry.Indices.push_back(i2);
		}
	}

	ModelParams.Model = _engine->video->LoadModel(geometry);
}
