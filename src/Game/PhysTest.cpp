#include "PhysTest.h"

static glm::dvec3 VecToGlm(const Math::Vec<3>& vec)
{
	return glm::dvec3(vec[0], vec[1], vec[2]);
}

static Math::Mat<4> GlmToMat(const glm::dmat4& mat)
{
	Math::Mat<4> res;

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			res[row][col] = mat[col][row];
		}
	}

	return res;
}

namespace PhysTest
{
	PhysTest::PhysTest(Common common) : _object(common)
	{
		_common = common;

		_surface.ModelParams.Model =
			_common.video->LoadModel(_surface.Geometry);

		auto texData = Loader::LoadImage("Images/White.png");
		_surface.TextureParams.SetAll(
			_common.video->LoadTexture(texData));

		_surface.ModelParams.Matrix = Math::Mat<4>(1.0);
		_surface.DrawParams.Enabled = true;

		_surface.PhysicalParams.Vertices = _surface.Geometry.Vertices;
		_surface.PhysicalParams.Normals = _surface.Geometry.Normals;
		_surface.PhysicalParams.Indices = _surface.Geometry.Indices;
		_surface.PhysicalParams.Matrix = Math::Mat<4>(1.0);
		_surface.PhysicalParams.Mu = 0.3;
		_surface.PhysicalParams.Bounciness = 0.1;
		_surface.PhysicalParams.Enabled = true;

		_common.video->RegisterModel(&_surface);
		_common.physicalEngine->RegisterObject(&_surface);

		_common.physicalEngine->RegisterObject(&_object);
		_common.universe->RegisterActor(&_object);
	}

	PhysTest::~PhysTest()
	{
		_common.video->RemoveModel(&_surface);
		_common.physicalEngine->RemoveObject(&_surface);

		_common.physicalEngine->RemoveObject(&_object);
		_common.universe->RemoveActor(&_object);

		_common.video->UnloadModel(_surface.ModelParams.Model);
		_common.video->UnloadTexture(_surface.TextureParams.Diffuse);
	}

	DynamicObject::DynamicObject(Common common)
	{
		_common = common;

		_cubeModel = _common.video->LoadModel(Loader::LoadModel(
			"Models/Ship/ShipFloorCollision.obj"));
		_cubeTexture = _common.video->LoadTexture(Loader::LoadImage(
			"Images/White.png"));

		for (size_t idx = 0; idx < 4; ++idx) {
			Model* cube = new Model;
			cube->ModelParams.Model = _cubeModel;
			cube->TextureParams.SetAll(_cubeTexture);
			cube->DrawParams.Enabled = true;
			cube->ModelParams.Matrix = Math::Mat<4>(1.0);
			cube->ModelParams.InnerMatrix = {GlmToMat(glm::scale(
				glm::dmat4(1.0),
				glm::dvec3(0.05, 0.05, 0.05)))};

			_common.video->RegisterModel(cube);
			_cubes.push_back(cube);

			SoftPhysicsValues::Vertex vertex;
			vertex.Mass = 2;
			vertex.Mu = 0.3;
			vertex.Bounciness = 0.1;
			vertex.Position = {
				(double)idx / 4,
				0.01 * idx * idx,
				(double)idx / 2.0 + 1.5
			};

			vertex.Force = {0, 0, -1};

			SoftPhysicsParams.Vertices.push_back(vertex);
		}

		SoftPhysicsValues::Link link;
		link.Index1 = 0;
		link.Index2 = 1;

		link.Length = 0.5;
		link.Friction = 5;
		link.K = 1000;

		SoftPhysicsParams.Links.push_back(link);

		link.Index1 = 1;
		link.Index2 = 2;
		SoftPhysicsParams.Links.push_back(link);

		link.Index1 = 0;
		link.Index2 = 2;
		SoftPhysicsParams.Links.push_back(link);

		link.Index1 = 0;
		link.Index2 = 3;
		SoftPhysicsParams.Links.push_back(link);

		link.Index1 = 1;
		link.Index2 = 3;
		SoftPhysicsParams.Links.push_back(link);

		link.Index1 = 2;
		link.Index2 = 3;
		SoftPhysicsParams.Links.push_back(link);
	}

	DynamicObject::~DynamicObject()
	{
		for (Model* cube : _cubes) {
			_common.video->RemoveModel(cube);
		}

		_common.video->UnloadModel(_cubeModel);
		_common.video->UnloadTexture(_cubeTexture);
	}

	void DynamicObject::Tick()
	{
		for (size_t idx = 0; idx < _cubes.size(); ++idx) {
			_cubes[idx]->ModelParams.Matrix = GlmToMat(
				glm::translate(
					glm::dmat4(1.0),
					VecToGlm(SoftPhysicsParams
						.Vertices[idx].Position)));
		}
	}
}
