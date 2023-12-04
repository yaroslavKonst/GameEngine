#include "world.h"

#include "../Utils/loader.h"
#include "../Utils/Text.h"
#include "player.h"
#include "ship.h"
#include "planet.h"
#include "../Logger/logger.h"
#include "../Assets/board.h"
#include "GravityField.h"

static void UniverseThread(Universe* universe)
{
	universe->MainLoop();
}

World::World()
{
	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 2;

	_video = new Video(1400, 1000, "Game", "Game", &videoSettings);
	_audio = new Audio;
	_universe = new Universe(10, _video);

	_universe->RegisterActor(this);

	_collisionEngine = new CollisionEngine();
	_universe->RegisterCollisionEngine(_collisionEngine);

	int skyboxWidth;
	int skyboxHeight;
	auto skyboxData = Loader::LoadImage(
		"Skybox/skybox.png",
		skyboxWidth,
		skyboxHeight);

	_video->CreateSkybox(skyboxWidth, skyboxHeight, skyboxData);
	_video->SetSkyboxColor({1, 1, 1});

	_video->SetFOV(80);
	_video->SetCameraUp({0, 0, 1});

	_textHandler = new TextHandler(_video->GetTextures());
	auto glyphs = Text::LoadFont(
		"Fonts/DroidSans.ttf",
		Text::DecodeUTF8(" -.:;ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]()"));

	_textHandler->LoadFont(glyphs);
}

World::~World()
{
	delete _textHandler;

	_universe->RemoveCollisionEngine(_collisionEngine);
	delete _collisionEngine;

	_universe->RemoveActor(this);

	delete _universe;
	delete _audio;
	delete _video;
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

	uint32_t testTexture = _video->GetTextures()->AddTexture(tw, th, td);

	Planet planet(20000, {0, 0, -20000}, _video, _collisionEngine);
	gf.AddObject({1000000000.0f, {0, 0, -20000}});

	Board board(_video);
	board.SetTexture({testTexture});
	board.SetModelHoled(true);

	_video->RegisterModel(&board);

	Light sun1;
	sun1.SetLightType(Light::Type::Point);
	sun1.SetLightColor({2000000, 2000000, 2000000});
	sun1.SetLightPosition({0, 0, 5000});
	sun1.SetLightActive(true);

	_video->RegisterLight(&sun1);

	Light sun2;
	sun2.SetLightType(Light::Type::Point);
	sun2.SetLightColor({2000000, 2000000, 2000000});
	sun2.SetLightPosition({0, 0, -45000});
	sun2.SetLightActive(true);

	_video->RegisterLight(&sun2);

	Sprite sprite1;
	sprite1.SetSpritePosition({10, 10, 1.8});
	sprite1.SetSpriteUp({0, 0, 1});
	sprite1.SetSpriteTexCoords({0, 0, 1, 1});
	sprite1.SetSpriteSize({1, 1});
	sprite1.SetDrawEnabled(true);
	sprite1.SetTexCount(1);
	sprite1.SetTexture(0, testTexture);

	_video->RegisterSprite(&sprite1);

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

	_video->RegisterSprite(&sprite2);

	Shuttle ship(_video, _collisionEngine, _textHandler, &gf);

	Player player(_video, _collisionEngine, &ship, _textHandler, &gf);
	_universe->RegisterActor(&player);
	_universe->RegisterActor(&ship);
	_collisionEngine->RegisterObject(&player);

	_universeThread = new std::thread(UniverseThread, _universe);

	_video->MainLoop();

	_universe->Stop();
	_universeThread->join();
	delete _universeThread;

	_collisionEngine->RemoveObject(&player);
	_universe->RemoveActor(&player);
	_universe->RemoveActor(&ship);

	_video->RemoveSprite(&sprite1);
	_video->RemoveSprite(&sprite2);

	_video->RemoveLight(&sun1);
	_video->RemoveLight(&sun2);

	_video->RemoveModel(&board);
}

void World::Tick()
{
}
