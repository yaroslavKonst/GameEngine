#include "world.h"

#include "../Utils/loader.h"
#include "player.h"
#include "ship.h"

class Field : public Model, public Object
{
public:
	Field(Video* video)
	{
		std::vector<glm::vec3> objectVertices = {
			{-200, -200, 0.0},
			{-200, 200, 0.0},
			{200, 200, 0.0},
			{200, -200, 0.0},
		};

		std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

		SetObjectVertices(objectVertices);
		SetObjectIndices(indices);
		SetObjectCenter({0.0f, 0.0f, -300.0f});
		SetObjectMatrix(glm::mat4(1.0));
		SetModelMatrix(
			glm::translate(
			glm::scale(
				glm::mat4(1.0),
				glm::vec3(10, 10, 1)),
				glm::vec3(0, 0, -0.251981)));
		SetModelInnerMatrix(glm::mat4(1.0));
		SetModelInstances({glm::mat4(1.0)});

		auto model = Loader::LoadModel(
			"../src/Assets/Resources/Models/field.obj");

		for (auto& coord : model.TexCoords) {
			coord *= 100;
		}

		SetModelVertices(model.Vertices);
		SetModelNormals(model.Normals);
		SetModelTexCoords(model.TexCoords);
		SetModelIndices(model.Indices);

		int texWidth;
		int texHeight;
		auto texData = Loader::LoadImage(
			"../src/Assets/Resources/Models/floor.jpg",
			texWidth,
			texHeight);

		uint32_t woodenTiles = video->GetTextures()->AddTexture(
			texWidth,
			texHeight,
			texData);

		SetTexture({woodenTiles});

		SetDrawEnabled(true);
	}
};

static void UniverseThread(Universe* universe)
{
	universe->MainLoop();
}

World::World()
{
	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 2;

	_video = new Video(1400, 1000, "Game", "Game", &videoSettings);
	_universe = new Universe(10);

	_video->SetSceneMutex(&_sceneMutex);
	_universe->SetSceneMutex(&_sceneMutex);

	_collisionEngine = new CollisionEngine();
	_universe->RegisterCollisionEngine(_collisionEngine);

	int skyboxWidth;
	int skyboxHeight;
	auto skyboxData = Loader::LoadImage(
		"../src/Assets/Resources/Skybox/skybox.png",
		skyboxWidth,
		skyboxHeight);

	_video->CreateSkybox(skyboxWidth, skyboxHeight, skyboxData);
	_video->SetSkyboxColor({1, 1, 1});

	_video->SetFOV(80);
	_video->SetCameraUp({0, 0, 1});

	int tw;
	int th;
	auto td = Loader::LoadImage(
		"../src/Assets/Resources/Models/wall.png",
		tw,
		th);

	_shipBlockTexture = _video->GetTextures()->AddTexture(
		tw,
		th,
		td);

	_universeThread = new std::thread(UniverseThread, _universe);
}

World::~World()
{
	_universe->Stop();
	_universeThread->join();
	delete _universeThread;

	delete _collisionEngine;
	delete _universe;
	delete _video;
}

void World::Run()
{
	Light sun;
	sun.SetLightType(Light::Type::Point);
	sun.SetLightColor({2000, 2000, 2000});
	sun.SetLightPosition({0, 0, 200});
	sun.SetLightActive(true);

	_video->RegisterLight(&sun);

	Ship ship(_video, _shipBlockTexture, _collisionEngine);
	ship.InsertBlock({-1, -1, 0}, {0, 0, 0});
	ship.InsertBlock({-1, 0, 0}, {0, 0, 0});
	ship.InsertBlock({-1, 1, 0}, {0, 0, 0});
	ship.InsertBlock({0, -1, 0}, {0, 0, 0});
	ship.InsertBlock({0, 0, 0}, {0, 0, 0});
	ship.InsertBlock({0, 1, 0}, {0, 0, 0});
	ship.InsertBlock({1, -1, 0}, {0, 0, 0});
	ship.InsertBlock({1, 0, 0}, {0, 0, 0});
	ship.InsertBlock({1, 1, 0}, {0, 0, 0});

	Player player(_video, _collisionEngine, &ship);
	_universe->RegisterActor(&player);
	_universe->RegisterActor(&ship);
	_collisionEngine->RegisterObject(&player);

	Field field(_video);
	_video->RegisterModel(&field);
	_collisionEngine->RegisterObject(&field);

	_video->MainLoop();

	_video->RemoveModel(&field);
	_collisionEngine->RemoveObject(&field);

	_collisionEngine->RemoveObject(&player);
	_universe->RemoveActor(&player);
	_universe->RemoveActor(&ship);

	_video->RemoveLight(&sun);
}
