#include "world.h"

#include "../Utils/loader.h"
#include "../Utils/Text.h"
#include "player.h"
#include "ship.h"
#include "planet.h"
#include "../Logger/logger.h"
#include "board.h"
#include "GravityField.h"

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
	_common.universe->RegisterCollisionEngine(_common.collisionEngine);

	int skyboxWidth;
	int skyboxHeight;
	auto skyboxData = Loader::LoadImage(
		"Skybox/skybox.png",
		skyboxWidth,
		skyboxHeight);

	_skyboxTexture = _common.video->CreateSkyboxTexture(
		skyboxWidth,
		skyboxHeight,
		skyboxData);
	_common.video->SetSkyboxTexture(_skyboxTexture);

	_common.video->SetFOV(80);
	_common.video->SetCameraUp({0, 0, 1});

	_common.localizer = new Localizer("Locale/en");

	_common.textHandler = new TextHandler(_common.video->GetTextures());
	auto glyphs = Text::LoadFont(
		"Fonts/DroidSans.ttf",
		_common.localizer->GetCharSet());

	_common.textHandler->LoadFont(glyphs);
}

World::~World()
{
	_common.video->DestroySkyboxTexture(_skyboxTexture);

	delete _common.textHandler;
	delete _common.localizer;

	_common.universe->RemoveCollisionEngine(_common.collisionEngine);
	delete _common.collisionEngine;

	_common.universe->RemoveActor(this);

	delete _common.universe;
	delete _common.audio;
	delete _common.video;
}

void World::Run()
{
	GravityField gf;

	int tw;
	int th;
	auto td = Loader::LoadImage(
		"Images/transparent.png",
		tw,
		th);

	uint32_t testTexture =
		_common.video->GetTextures()->AddTexture(tw, th, td);

	Planet planet(
		20000,
		{0, 0, -20000},
		_common.video,
		_common.collisionEngine);
	gf.AddObject({1000000000.0f, {0, 0, -20000}});

	Board board(_common.video);
	board.SetTexture({testTexture});
	board.SetModelHoled(true);

	_common.video->RegisterModel(&board);

	Light sun1;
	sun1.SetLightType(Light::Type::Point);
	sun1.SetLightColor({2000000, 2000000, 2000000});
	sun1.SetLightPosition({0, 0, 5000});
	sun1.SetLightActive(true);

	_common.video->RegisterLight(&sun1);

	Light sun2;
	sun2.SetLightType(Light::Type::Point);
	sun2.SetLightColor({2000000, 2000000, 2000000});
	sun2.SetLightPosition({0, 0, -45000});
	sun2.SetLightActive(true);

	_common.video->RegisterLight(&sun2);

	Sprite sprite1;
	sprite1.SetSpritePosition({10, 10, 1.8});
	sprite1.SetSpriteUp({0, 0, 1});
	sprite1.SetSpriteTexCoords({0, 0, 1, 1});
	sprite1.SetSpriteSize({1, 1});
	sprite1.SetDrawEnabled(true);
	sprite1.SetTexCount(1);
	sprite1.SetTexture(0, testTexture);

	_common.video->RegisterSprite(&sprite1);

	Sprite sprite2;
	sprite2.SetSpritePosition({10, 0, 1.8});
	sprite2.SetSpriteUp({0, 0, 1});
	sprite2.SetSpriteTexCoords({0, 0, 1, 1});
	sprite2.SetSpriteSize({1, 1});
	sprite2.SetDrawEnabled(true);
	sprite2.SetTexCount(1);
	sprite2.SetTexture(0, testTexture);
	sprite2.SetColorMultiplier({40, 40, 40, 1});
	sprite2.SetDrawLight(true);

	_common.video->RegisterSprite(&sprite2);

	Shuttle ship(_common, &gf);

	Player player(_common, &ship, &gf);
	_common.universe->RegisterActor(&player);
	_common.universe->RegisterActor(&ship);
	_common.collisionEngine->RegisterObject(&player);

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
}
