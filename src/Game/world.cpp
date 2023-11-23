#include "world.h"

#include "../Utils/loader.h"
#include "../Utils/Text.h"
#include "player.h"
#include "ship.h"
#include "planet.h"
#include "../Logger/logger.h"
#include "../Assets/board.h"

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

		std::vector<glm::vec3> objectNormals = {
			{0, 0, 1.0},
			{0, 0, 1.0},
			{0, 0, 1.0},
			{0, 0, 1.0},
		};


		std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

		SetObjectVertices(objectVertices);
		SetObjectNormals(objectNormals);
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

		auto model = Loader::LoadModel(
			"Models/Archive/field.obj");

		for (auto& coord : model.TexCoords) {
			coord *= 100;
		}

		_video = video;
		_model = _video->LoadModel(model);
		SetModels({_model});

		int texWidth;
		int texHeight;
		auto texData = Loader::LoadImage(
			"Models/Archive/floor.jpg",
			texWidth,
			texHeight);

		_woodenTiles = video->GetTextures()->AddTexture(
			texWidth,
			texHeight,
			texData);

		SetTexture({_woodenTiles});

		SetDrawEnabled(true);

		_rectangle = new Rectangle();
		_rectangle->SetRectangleTexCoords({0, 0, 1, 1});
		_rectangle->SetRectangleDepth(0);
		_rectangle->SetTexture({_woodenTiles});
		_rectangle->SetRectanglePosition({-0.5, -0.5, -0.4, -0.4});
		_rectangle->SetDrawEnabled(true);

		_video->RegisterRectangle(_rectangle);
	}

	~Field()
	{
		_video->RemoveRectangle(_rectangle);
		delete _rectangle;

		_video->UnloadModel(_model);
		_video->GetTextures()->RemoveTexture(_woodenTiles);
	}

private:
	Video* _video;
	uint32_t _model;
	uint32_t _woodenTiles;

	Rectangle* _rectangle;
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

	_universeThread = new std::thread(UniverseThread, _universe);

	_textHandler = new TextHandler(_video->GetTextures());
	auto glyphs = Text::LoadFont(
		"Fonts/DroidSans.ttf",
		Text::DecodeUTF8(" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]()"));

	_textHandler->LoadFont(glyphs);
}

World::~World()
{
	_universe->Stop();
	_universeThread->join();
	delete _universeThread;

	delete _textHandler;

	delete _collisionEngine;

	_universe->RemoveActor(this);

	delete _universe;
	delete _audio;
	delete _video;
}

void World::Run()
{
	int tw;
	int th;
	auto td = Loader::LoadImage(
		"Images/transparent.png",
		tw,
		th);

	uint32_t testTexture = _video->GetTextures()->AddTexture(tw, th, td);

	Planet planet(1, _video);

	Board board(_video);
	board.SetTexture({testTexture});
	board.SetModelHoled(true);

	_video->RegisterModel(&board);

	Light sun;
	sun.SetLightType(Light::Type::Point);
	sun.SetLightColor({200, 200, 200});
	sun.SetLightPosition({0, 0, 50});
	sun.SetLightActive(true);

	_video->RegisterLight(&sun);

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

	Shuttle ship(_video, _collisionEngine);

	Player player(_video, _collisionEngine, &ship, _textHandler);
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

	_video->RemoveSprite(&sprite1);
	_video->RemoveSprite(&sprite2);

	_video->RemoveLight(&sun);

	_video->RemoveModel(&board);
}

void World::Tick()
{
}
