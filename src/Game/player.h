#ifndef _PLAYER_H
#define _PLAYER_H

#include <algorithm>

#include "../VideoEngine/video.h"
#include "../VideoEngine/GUI/TextBox.h"
#include "../VideoEngine/GUI/button.h"
#include "../UniverseEngine/universe.h"
#include "../Utils/loader.h"
#include "../Assets/animation.h"
#include "ship.h"
#include "shuttle.h"
#include "GravityField.h"

class Player : public Actor, public Object, public InputHandler
{
public:
	Player(
		Video* video,
		CollisionEngine* rayEngine,
		Shuttle* ship,
		TextHandler* textHandler,
		GravityField* gf);
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
	GravityField* _gf;

	glm::vec3 _pos;
	glm::vec3 _dirUp;
	glm::vec3 _dirF;
	glm::vec3 _dirR;

	float _angleH;
	float _angleV;
	glm::vec3 _speed;

	int _go;
	int _strafe;
	bool _jump;
	Light _light;
	bool _lightActive;

	Shuttle* _ship;
	bool _buildMode;
	bool _flightMode;
	float _buildCamCoeff;
	glm::vec3 _buildCamPos;

	bool _actionERequested;
	bool _actionRRequested;
	bool _actionFRequested;

	TextBox* _centerTextBox;
	TextBox* _cornerTextBox;

	Rectangle _cross;
	uint32_t _crossTexture;

	FlightControl* _activeFlightControl;
};

#endif
