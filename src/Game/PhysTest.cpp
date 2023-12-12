#include "PhysTest.h"

namespace PhysTest
{
	PhysTest::PhysTest(Common common) : _object(common)
	{
		_common = common;

		_surface.ModelParams.Model =
			_common.video->LoadModel(_surface.Geometry);

		auto texData = Loader::LoadImage("Images/White.png");
		_surface.TextureParams.SetAll(
			_common.video->AddTexture(texData));

		_surface.ModelParams.Matrix = glm::mat4(1.0f);
		_surface.DrawParams.Enabled = true;

		_surface.PhysicalParams.Vertices = _surface.Geometry.Vertices;
		_surface.PhysicalParams.Normals = _surface.Geometry.Normals;
		_surface.PhysicalParams.Indices = _surface.Geometry.Indices;
		_surface.PhysicalParams.Matrix = glm::mat4(1.0f);
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
		_common.video->RemoveTexture(_surface.TextureParams.Diffuse);
	}

	DynamicObject::DynamicObject(Common common)
	{
		_common = common;

		_cubeModel = _common.video->LoadModel(Loader::LoadModel(
			"Models/Ship/ShipFloorCollision.obj"));
		_cubeTexture = _common.video->AddTexture(Loader::LoadImage(
			"Images/White.png"));

		for (size_t idx = 0; idx < 2; ++idx) {
			Model* cube = new Model;
			cube->ModelParams.Model = _cubeModel;
			cube->TextureParams.SetAll(_cubeTexture);
			cube->DrawParams.Enabled = true;
			cube->ModelParams.Matrix = glm::mat4(1.0);
			cube->ModelParams.InnerMatrix = glm::scale(
				glm::mat4(1.0),
				glm::vec3(0.05, 0.05, 0.05));

			_common.video->RegisterModel(cube);
			_cubes.push_back(cube);

			SoftPhysicsValues::Vertex vertex;
			vertex.Mass = 1;
			vertex.Mu = 0.3;
			vertex.Bounciness = 0.1;
			vertex.Position = {(float)idx / 4, 0, 3};

			vertex.Force = {0, 0, -1};

			SoftPhysicsParams.Vertices.push_back(vertex);
		}

		SoftPhysicsValues::Link link;
		link.Index1 = 0;
		link.Index2 = 1;

		link.Length = 0.5;
		link.Friction = 0.1;
		link.K = 100;

		//SoftPhysicsParams.Links.push_back(link);
	}

	DynamicObject::~DynamicObject()
	{
		for (Model* cube : _cubes) {
			_common.video->RemoveModel(cube);
		}

		_common.video->UnloadModel(_cubeModel);
		_common.video->RemoveTexture(_cubeTexture);
	}

	void DynamicObject::Tick()
	{
		for (size_t idx = 0; idx < _cubes.size(); ++idx) {
			_cubes[idx]->ModelParams.Matrix = glm::translate(
				glm::mat4(1.0),
				SoftPhysicsParams.Vertices[idx].Position);
		}
	}
}
