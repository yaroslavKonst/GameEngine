#include "world.h"

#include "../Utils/loader.h"
#include "../Utils/Text.h"
#include "player.h"
#include "ship.h"
#include "planet.h"
#include "../Logger/logger.h"
#include "board.h"
#include "GravityField.h"
#include "PhysTest.h"

static void UniverseThread(Universe* universe)
{
	universe->MainLoop();
}

World::World()
{
	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 2;

	_common.video = new Video(1400, 1000, "Game", "Game", &videoSettings);
	_common.audio = new Audio;
	_common.universe = new Universe(10, _common.video);

	_common.universe->RegisterActor(this);

	_common.collisionEngine = new CollisionEngine();
	_common.universe->RegisterPhysicalEngine(_common.collisionEngine);

	_common.physicalEngine = new PhysicalEngine();
	_common.universe->RegisterPhysicalEngine(_common.physicalEngine);

	_common.video->SetSkyboxNumber(2);

	auto skyboxData = Loader::LoadImage("Skybox/skybox.png");
	_skyboxDay = _common.video->AddSkyboxTexture(skyboxData);
	_common.video->SetSkyboxEnabled(true);
	_common.video->SetSkyboxTexture(_skyboxDay);

	skyboxData = Loader::LoadImage("Skybox/skybox_night.png");
	_skyboxNight = _common.video->AddSkyboxTexture(skyboxData);
	_common.video->SetSkyboxEnabled(true, 1);
	_common.video->SetSkyboxTexture(_skyboxNight, 1);

	_common.video->SetFOV(80);
	_common.video->SetCameraUp({0, 0, 1});

	_common.localizer = new Localizer("Locale/en");

	auto glyphs = Text::LoadFont(
		"Fonts/DroidSans.ttf",
		_common.localizer->GetCharSet());

	_common.textHandler = new TextHandler(_common.video, glyphs);

	_skyboxTime = 0;
}

World::~World()
{
	_common.video->RemoveTexture(_skyboxDay);
	_common.video->RemoveTexture(_skyboxNight);

	delete _common.textHandler;
	delete _common.localizer;

	_common.universe->RemovePhysicalEngine(_common.collisionEngine);
	delete _common.collisionEngine;

	_common.universe->RemovePhysicalEngine(_common.physicalEngine);
	delete _common.physicalEngine;

	_common.universe->RemoveActor(this);

	delete _common.universe;
	delete _common.audio;
	delete _common.video;
}

void World::Run()
{
	GravityField gf;

	auto td = Loader::LoadImage("Images/transparent.png");
	uint32_t testTexture = _common.video->AddTexture(td);

	Planet planet(
		20000,
		{0, 0, -20000},
		_common.video,
		_common.collisionEngine);
	gf.AddObject({1000000000.0f, {0, 0, -20000}});

	Board board(_common.video);
	board.TextureParams.SetAll(testTexture);
	board.ModelParams.Holed = true;

	_common.video->RegisterModel(&board);

	Light sun1;
	sun1.Type = Light::Type::Point;
	sun1.Color = {2000000, 2000000, 2000000};
	sun1.Position = {0, 0, 5000};
	sun1.Enabled = true;

	_common.video->RegisterLight(&sun1);

	Light sun2;
	sun2.Type = Light::Type::Point;
	sun2.Color = {2000000, 2000000, 2000000};
	sun2.Position = {0, 0, -45000};
	sun2.Enabled = true;

	_common.video->RegisterLight(&sun2);

	Sprite sprite1;
	sprite1.SpriteParams.Position = {10, 10, 1.8};
	sprite1.SpriteParams.Up = {0, 0, 1};
	sprite1.SpriteParams.Size = {1, 1};
	sprite1.DrawParams.Enabled = true;
	sprite1.TextureParams.SetAll(testTexture);

	_common.video->RegisterSprite(&sprite1);

	Sprite sprite2;
	sprite2.SpriteParams.Position = {10, 0, 1.8};
	sprite2.SpriteParams.Up = {0, 0, 1};
	sprite2.SpriteParams.Size = {1, 1};
	sprite2.DrawParams.Enabled = true;
	sprite2.TextureParams.SetAll(testTexture);
	sprite2.DrawParams.ColorMultiplier = {40, 40, 40, 1};
	sprite2.TextureParams.IsLight = true;

	_common.video->RegisterSprite(&sprite2);

	Shuttle ship(_common, &gf);

	Player player(_common, &ship, &gf, &planet);
	_common.universe->RegisterActor(&player);
	_common.universe->RegisterActor(&ship);
	_common.collisionEngine->RegisterObject(&player);

	PhysTest::PhysTest physTest(_common);

	_universeThread = new std::thread(UniverseThread, _common.universe);

	_common.video->MainLoop();

	_common.universe->Stop();
	_universeThread->join();
	delete _universeThread;

	_common.collisionEngine->RemoveObject(&player);
	_common.universe->RemoveActor(&player);
	_common.universe->RemoveActor(&ship);

	_common.video->RemoveSprite(&sprite1);
	_common.video->RemoveSprite(&sprite2);

	_common.video->RemoveLight(&sun1);
	_common.video->RemoveLight(&sun2);

	_common.video->RemoveModel(&board);
}

void World::Tick()
{
	_skyboxTime += 0.001;

	if (_skyboxTime >= M_PI * 2.0) {
		_skyboxTime = 0;
	}

	float gx = sin(_skyboxTime);
	float gy = cos(_skyboxTime);
	glm::vec3 gradient1 = glm::normalize(glm::vec3(gx, gy, 0.2f)) * 2.0f;
	glm::vec3 gradient2 = -gradient1;

	float offset = sin(_skyboxTime * 4) * 3.0;

	_common.video->SetSkyboxGradient(true, gradient1, offset, 0);
	_common.video->SetSkyboxGradient(true, gradient2, -offset, 1);
}
