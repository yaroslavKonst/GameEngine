#ifndef _PLAYER_H
#define _PLAYER_H

#include <algorithm>

#include "../VideoEngine/video.h"
#include "../VideoEngine/GUI/TextBox.h"
#include "../UniverseEngine/universe.h"
#include "../Utils/loader.h"
#include "../Assets/animation.h"
#include "ship.h"

class Player : public Actor, public Object, public InputHandler
{
public:
	Player(
		Video* video,
		CollisionEngine* rayEngine,
		Ship* ship,
		TextHandler* textHandler);
	~Player();

	void Key(
		int key,
		int scancode,
		int action,
		int mods) override;

	bool MouseMoveRaw(
		double xoffset,
		double yoffset) override;

	void Tick() override;
	void TickEarly() override;

	void BuildActions(int key, int action);

private:
	Video* _video;
	CollisionEngine* _rayEngine;
	TextHandler* _textHandler;

	glm::vec3 _pos;
	float _angleH;
	float _angleV;
	float _vspeed;

	int _go;
	int _strafe;
	bool _jump;
	Light _light;
	bool _lightActive;

	std::mutex _mutex;

	Ship* _ship;
	bool _buildMode;
	glm::ivec3 _buildPos;
	glm::vec3 _buildRotation;
	float _buildCamCoeff;
	glm::vec3 _buildCamPos;

	bool _actionRequested;

	TextBox* _centerTextBox;
	TextBox* _cornerTextBox;

	Rectangle _cross;
	uint32_t _crossTexture;
};

#endif
