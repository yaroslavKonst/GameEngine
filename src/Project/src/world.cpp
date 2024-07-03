#include "world.h"

#include "../../Engine/Utils/loader.h"
#include "../../Engine/Utils/Text.h"
#include "../../Engine/Utils/TextFileParser.h"
#include "../../Engine/Logger/logger.h"

World::World(Engine* engine, std::function<void()> endCallback)
{
	_engine = engine;
	_endCallback = endCallback;

	_ticksBetweenBits = 40;
	_ticksToBit = _ticksBetweenBits;

	_win = false;

	_loaded = false;
	_gameGlobal.Paused = true;
	LoadResources();

	_infoTextBox = new TextBox(_engine->video, _engine->textHandler);
	_infoTextBox->SetPosition(0, -0.5, TextBox::Alignment::Center);
	_infoTextBox->SetTextSize(0.05);
	_infoTextBox->SetText(_engine->localizer->Localize("Paused"));
	_infoTextBox->SetTextColor({1, 1, 1, 1});
	_infoTextBox->SetDepth(0);

	size_t bufLen = Audio::SampleRate / 4;
	_audioBitBuffer.Data.resize(bufLen * 2);
	float freq = 440;

	for (size_t i = 0 ; i < bufLen; ++i) {
		float value =
			std::min<float>(1.0, (float)i / 500.0) *
			sin((float)i / Audio::SampleRate * 2.0 * M_PI * freq) *
			(1.0 - (float)i / bufLen);

		_audioBitBuffer.Data[i * 2] = value;
		_audioBitBuffer.Data[i * 2 + 1] = value;
	}

	_audioBitBuffer.Finished = true;
	_audioBitBuffer.Multiplier = 0.2;

	_engine->universe->RegisterActor(this);
}

World::~World()
{
	Unload();

	_engine->universe->RemoveActor(this);

	_audioBitBuffer.Discard = true;

	while (!_audioBitBuffer.Finished)
	{
		usleep(1000);
	}

	_infoTextBox->Deactivate();
	delete _infoTextBox;

	Unload();
	UnloadResources();
}

void World::LoadResources()
{
	auto image = Loader::LoadImage("World/Textures/Skybox.png");
	_skybox = _engine->video->LoadSkyboxTexture(image);
}

void World::UnloadResources()
{
	_engine->video->UnloadTexture(_skybox);
}

void World::Load()
{
	if (_loaded) {
		Unload();
	}

	_engine->video->SetSkyboxNumber(1);
	_engine->video->SetSkyboxTexture(_skybox);
	_engine->video->SetSkyboxEnabled(true);

	_player = new Player(
		_engine,
		&_gameGlobal,
		[this](double x, double y) -> void {SetPlayerPosition(x, y);});

	_playerX = 20000;
	_playerY = 20000;

	_loaded = true;
	_gameGlobal.Paused = false;

	_threadWork = true;
	_loaderThread = new std::thread([this]() -> void {LoaderThread();});
}

void World::Unload()
{
	if (!_loaded) {
		return;
	}

	_threadWork = false;
	_loaderThread->join();
	delete _loaderThread;

	_engine->video->SetSkyboxNumber(0);

	_infoTextBox->Deactivate();

	_loaded = false;
	_gameGlobal.Paused = true;

	delete _player;

	for (auto& region : _map) {
		delete region.second;
	}
}

void World::Tick(double time)
{
	if (!_loaded) {
		return;
	}

	if (_gameGlobal.Paused) {
		return;
	}

	--_ticksToBit;

	if (_ticksToBit == 0) {
		_ticksToBit = _ticksBetweenBits;
		_engine->audio->Submit(&_audioBitBuffer);
	}

	if (_win) {
		_win = false;
		_endCallback();
	}
}

void World::SetPlayerPosition(double x, double y)
{
	_newX = floor(x / 0.5 / 128.0 + 0.5);
	_newY = floor(y / 0.5 / 128.0 + 0.5);
}

void World::LoaderThread()
{
	const int lim = 7;

	while (_threadWork) {
		if (_playerX == _newX && _playerY == _newY) {
			usleep(20000);
			continue;
		}

		_playerX = _newX;
		_playerY = _newY;

		std::set<Coord> tilesToRemove;

		for (auto& tile : _map) {
			bool removeTile =
				abs(tile.first.X - _playerX) > lim ||
				abs(tile.first.Y - _playerY) > lim;

			if (removeTile) {
				tilesToRemove.insert(tile.first);
			}
		}

		for (Coord tile : tilesToRemove) {
			delete _map[tile];
			_map.erase(tile);
		}

		for (int x = _playerX - lim; x <= _playerX + lim; ++x) {
			for (int y = _playerY - lim; y <= _playerY + lim; ++y) {
				if (_map.find({x, y}) == _map.end()) {
					_map[{x, y}] = new WorldRegion(
						_engine,
						x,
						y);
				}

				int lod = std::max<int>(
					abs(x - _playerX),
					abs(y - _playerY)) - 1;

				lod = std::clamp<int>(lod, 0, 6);

				_map[{x, y}]->Load(lod);
			}
		}
	}
}
