#include "WorldRegion.h"

#include "../../Engine/Math/transform.h"

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

void WorldRegion::Load()
{
	if (_loaded) {
		return;
	}

	auto textureData = Loader::LoadImage("World/Textures/Grass.png");
	TextureParams.SetAll(_engine->video->LoadTexture(textureData));

	BuildSurface();

	ModelParams.Matrix = Math::Translate({
		(_x * CellCount - (double)CellCount / 2.0) * CellSize,
		(_y * CellCount - (double)CellCount / 2.0) * CellSize,
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

	for (int vy = 0; vy <= CellCount; ++vy) {
		for (int vx = 0; vx <= CellCount; ++vx) {
			Math::Vec<3> vertex = {
				(double)vx * CellSize,
				(double)vy * CellSize,
				0.0
			};

			Math::Vec<3> normal = {0.0, 0.0, 1.0};

			Math::Vec<2> texCoord = {
				(double)vx * CellSize,
				(double)vy * CellSize
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

	for (int vy = 0; vy < CellCount; ++vy) {
		for (int vx = 0; vx < CellCount; ++vx) {
			uint32_t i0 = vy * (CellCount + 1) + vx;
			uint32_t i1 = vy * (CellCount + 1) + vx + 1;
			uint32_t i2 = (vy + 1) * (CellCount + 1) + vx;

			geometry.Indices.push_back(i0);
			geometry.Indices.push_back(i1);
			geometry.Indices.push_back(i2);

			i0 = (vy + 1) * (CellCount + 1) + vx;
			i1 = vy * (CellCount + 1) + vx + 1;
			i2 = (vy + 1) * (CellCount + 1) + vx + 1;

			geometry.Indices.push_back(i0);
			geometry.Indices.push_back(i1);
			geometry.Indices.push_back(i2);
		}
	}

	ModelParams.Model = _engine->video->LoadModel(geometry);
}
