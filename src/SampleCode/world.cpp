#include "world.h"

#include "../Engine/Utils/loader.h"
#include "../Engine/Utils/Text.h"
#include "../Engine/Utils/TextFileParser.h"
#include "../Engine/Logger/logger.h"

World::World(Common* common, std::function<void()> endCallback)
{
	_common = common;
	_endCallback = endCallback;

	_ticksBetweenBits = 80;
	_ticksToBit = _ticksBetweenBits;

	_win = false;

	_loaded = false;
	_paused = true;
	LoadResources();

	_infoTextBox = new TextBox(_common->video, _common->textHandler);
	_infoTextBox->SetPosition(0, -0.5, TextBox::Alignment::Center);
	_infoTextBox->SetTextSize(0.05);
	_infoTextBox->SetText(_common->localizer->Localize("Paused"));
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

	_common->universe->RegisterActor(this);

	SetInputLayer(11);
	_common->video->Subscribe(this);
	SetInputEnabled(true);
}

World::~World()
{
	SetInputEnabled(false);
	_common->video->Unsubscribe(this);

	_common->universe->RemoveActor(this);

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
	auto image = Loader::LoadImage(
		"World/Player.png");
	_playerTexture = _common->video->LoadTexture(image);

	image = Loader::LoadImage(
		"World/EmptyCell.png");
	_emptyCellTexture = _common->video->LoadTexture(image);

	image = Loader::LoadImage(
		"World/WallCell.png");
	_wallCellTexture = _common->video->LoadTexture(image);

	image = Loader::LoadImage(
		"World/FinishCell.png");
	_finishCellTexture = _common->video->LoadTexture(image);
}

void World::UnloadResources()
{
	_common->video->UnloadTexture(_playerTexture);
	_common->video->UnloadTexture(_emptyCellTexture);
	_common->video->UnloadTexture(_wallCellTexture);
	_common->video->UnloadTexture(_finishCellTexture);
}

void World::Load()
{
	if (_loaded) {
		Unload();
	}

	_posX = 0;
	_posY = 0;
	_reqPosX = 0;
	_reqPosY = 0;

	_light = new Light();
	_light->Type = Light::Type::Point;
	_light->Color = {10, 10, 10};
	_light->Position = {0, 0, 10};
	_light->Enabled = true;
	_common->video->RegisterLight(_light);

	_map = LoadMap("World/Levels/Level.txt");

	_player = new Sprite();

	_player->SpriteParams.Position =
		{(double)_posX, (double)_posY, 1.0};
	_player->SpriteParams.Up = {0, 0, 1};
	_player->SpriteParams.Size = {1, 1};
	_player->DrawParams.Enabled = true;
	_player->TextureParams.SetAll(_playerTexture);
	_common->video->RegisterSprite(_player);

	_loaded = true;
	_paused = false;
}

void World::Unload()
{
	if (!_loaded) {
		return;
	}

	_infoTextBox->Deactivate();

	_loaded = false;
	_paused = true;

	_common->video->RemoveSprite(_player);
	delete _player;

	UnloadMap(_map);

	_common->video->RemoveLight(_light);
	delete _light;
}

std::map<World::Coord, World::Cell>* World::LoadMap(std::string name)
{
	auto mapFile = TextFileParser::ParseFile(name);

	std::vector<std::vector<Cell::Type>> cells;

	for (auto line : mapFile) {
		if (line[0] == "start") {
			int startX = std::stoi(line[1]);
			int startY = std::stoi(line[2]);

			_posX = startX;
			_posY = startY;
			_reqPosX = startX;
			_reqPosY = startY;
		} else {
			std::vector<Cell::Type> row;

			for (char c : line[0]) {
				switch (c) {
				case 'e':
					row.push_back(Cell::Type::Empty);
					break;
				case 'w':
					row.push_back(Cell::Type::Wall);
					break;
				case 'f':
					row.push_back(Cell::Type::Finish);
					break;
				default:
					Logger::Error() <<
						"Unknown char in map.";
					break;
				}
			}

			cells.push_back(row);
		}
	}

	std::map<Coord, Cell>* map = new std::map<Coord, Cell>();

	for (size_t x = 0; x < cells[0].size(); ++x) {
		for (size_t y = 0; y < cells.size(); ++y) {
			Cell cell;

			cell.sprite = new Sprite();
			cell.sprite->SpriteParams.Up = {0, 0, 1};
			cell.sprite->SpriteParams.Size = {1, 1};
			cell.sprite->DrawParams.Enabled = true;
			cell.type = cells[y][x];

			switch (cell.type) {
			case Cell::Type::Empty:
				cell.sprite->TextureParams.SetAll(
					_emptyCellTexture);
				cell.sprite->SpriteParams.Position =
					{(double)x, (double)y, 0.0};
				break;
			case Cell::Type::Wall:
				cell.sprite->TextureParams.SetAll(
					_wallCellTexture);
				cell.sprite->SpriteParams.Position =
					{(double)x, (double)y, 0.5};
				break;
			case Cell::Type::Finish:
				cell.sprite->TextureParams.SetAll(
					_finishCellTexture);
				cell.sprite->SpriteParams.Position =
					{(double)x, (double)y, 0.2};
				break;
			}

			_common->video->RegisterSprite(cell.sprite);
			map->operator[]({(int)x, (int)y}) = cell;
		}
	}

	return map;
}

void World::UnloadMap(std::map<World::Coord, World::Cell>* map)
{
	for (auto& item : *map) {
		Cell& cell = item.second;

		_common->video->RemoveSprite(cell.sprite);
		delete cell.sprite;
	}

	delete map;
}

void World::Tick()
{
	if (!_loaded) {
		return;
	}

	if (_paused) {
		return;
	}

	float step = 0.4;

	if (_posX < (double)_reqPosX) {
		if (fabs(_posX - (double)_reqPosX) < step) {
			_posX = (double)_reqPosX;
		} else {
			_posX += step;
		}
	} else if (_posX > (double)_reqPosX) {
		if (fabs(_posX - (double)_reqPosX) < step) {
			_posX = (double)_reqPosX;
		} else {
			_posX -= step;
		}
	}

	if (_posY < (double)_reqPosY) {
		if (fabs(_posY - (double)_reqPosY) < step) {
			_posY = (double)_reqPosY;
		} else {
			_posY += step;
		}
	} else if (_posY > (double)_reqPosY) {
		if (fabs(_posY - (double)_reqPosY) < step) {
			_posY = (double)_reqPosY;
		} else {
			_posY -= step;
		}
	}

	_common->video->SetCameraPosition(
		{_posX - 3.0, _posY - 0.2, 5.0});
	_common->video->SetCameraTarget({_posX, _posY, 0.0});
	_common->video->SetCameraUp({0, 0, 1});

	_light->Position = {(double)_posX, (double)_posY, 10.0};
	_player->SpriteParams.Position = {(double)_posX, (double)_posY, 1.0};

	--_ticksToBit;

	if (_ticksToBit == 0) {
		_ticksToBit = _ticksBetweenBits;
		_common->audio->Submit(&_audioBitBuffer);
	}

	if (_win) {
		_win = false;
		_endCallback();
	}
}

void World::Key(int key, int scancode, int action, int mods)
{
	if (_paused) {
		return;
	}

	if (action != GLFW_PRESS) {
		return;
	}

	bool rightTick = _ticksBetweenBits - _ticksToBit < 10 ||
		_ticksToBit < 5;

	if (!rightTick) {
		return;
	}

	int newX = _reqPosX;
	int newY = _reqPosY;

	if (key == GLFW_KEY_W) {
		newX = _reqPosX + 1;
	} else if (key == GLFW_KEY_S) {
		newX = _reqPosX - 1;
	} else if (key == GLFW_KEY_D) {
		newY = _reqPosY - 1;
	} else if (key == GLFW_KEY_A) {
		newY = _reqPosY + 1;
	}

	if (_map->find({newX, newY}) == _map->end()) {
		return;
	}

	if (_map->operator[]({newX, newY}).type == Cell::Type::Finish) {
		_win = true;
	}

	if (_map->operator[]({newX, newY}).type != Cell::Type::Empty) {
		return;
	}

	_reqPosX = newX;
	_reqPosY = newY;
}
