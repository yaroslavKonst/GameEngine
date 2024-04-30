#ifndef _WORLD_H
#define _WORLD_H

#include "../../Engine/Video/GUI/TextBox.h"
#include "global.h"
#include "GameGlobal.h"
#include "player.h"
#include "WorldRegion.h"

class World : public Actor
{
public:
	struct Coord
	{
		int X;
		int Y;

		bool operator<(const Coord& coord) const
		{
			if (X != coord.X) {
				return X < coord.X;
			}

			return Y < coord.Y;
		}
	};

	World(Engine* engine, std::function<void()> endCallback);
	~World();

	void Tick(double time) override;

	void Load();
	void Unload();

	void Pause()
	{
		_engine->video->ToggleRawMouseInput();

		_gameGlobal.Paused = true;
		_infoTextBox->Activate();
	}

	void Continue()
	{
		_infoTextBox->Deactivate();
		_gameGlobal.Paused = false;

		_engine->video->ToggleRawMouseInput();
	}

private:
	Engine* _engine;
	GameGlobal _gameGlobal;

	TextBox* _infoTextBox;

	Player* _player;

	std::atomic<bool> _loaded;

	void LoadResources();
	void UnloadResources();

	std::set<WorldRegion*> _map;

	Audio::Buffer _audioBitBuffer;
	size_t _ticksBetweenBits;
	size_t _ticksToBit;

	uint32_t _skybox;

	bool _win;
	std::function<void()> _endCallback;
};

#endif
