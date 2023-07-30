#include "world.h"

#include "../Utils/loader.h"
#include "player.h"
#include "ship.h"

static void UniverseThread(Universe* universe)
{
	universe->MainLoop();
}

World::World()
{
	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 2;

	_video = new Video(1400, 1000, "Game", "Game", &videoSettings);
	_universe = new Universe(5);

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
	Ship ship(_video, _shipBlockTexture);
	ship.InsertBlock({-1, -1, 0});
	ship.InsertBlock({-1, 0, 0});
	ship.InsertBlock({-1, 1, 0});
	ship.InsertBlock({0, -1, 0});
	ship.InsertBlock({0, 0, 0});
	ship.InsertBlock({0, 1, 0});
	ship.InsertBlock({1, -1, 0});
	ship.InsertBlock({1, 0, 0});
	ship.InsertBlock({1, 1, 0});

	Player player(_video, _collisionEngine, &ship);
	_universe->RegisterActor(&player);
	_collisionEngine->RegisterObject(&player);

	_video->MainLoop();

	_collisionEngine->RemoveObject(&player);
	_universe->RemoveActor(&player);
}
