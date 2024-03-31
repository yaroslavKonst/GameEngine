#ifndef _WORLD_H
#define _WORLD_H

#include "../Engine/Video/GUI/TextBox.h"
#include "global.h"

class World : public Actor, public InputHandler
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

	struct Cell
	{
		enum class Type
		{
			Empty,
			Wall,
			Finish
		};

		Type type;
		Sprite* sprite;
	};

	World(Common* common, std::function<void()> endCallback);
	~World();

	void Tick() override;

	void Load();
	void Unload();

	void Pause()
	{
		_paused = true;
		_infoTextBox->Activate();
	}

	void Continue()
	{
		_infoTextBox->Deactivate();
		_paused = false;
	}

	bool InInputArea(float x, float y) override
	{
		return true;
	}

	void Key(int key, int scancode, int action, int mods) override;

private:
	Common* _common;

	TextBox* _infoTextBox;

	double _posX;
	double _posY;
	int _reqPosX;
	int _reqPosY;
	Sprite* _player;

	std::atomic<bool> _loaded;
	std::atomic<bool> _paused;

	Light* _light;

	uint32_t _playerTexture;
	uint32_t _emptyCellTexture;
	uint32_t _wallCellTexture;
	uint32_t _finishCellTexture;
	void LoadResources();
	void UnloadResources();

	std::map<Coord, Cell>* _map;
	std::map<Coord, Cell>* LoadMap(std::string name);
	void UnloadMap(std::map<Coord, Cell>* map);

	Audio::Buffer _audioBitBuffer;
	size_t _ticksBetweenBits;
	size_t _ticksToBit;

	bool _win;
	std::function<void()> _endCallback;
};

#endif
