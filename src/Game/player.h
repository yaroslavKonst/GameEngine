#ifndef _PLAYER_H
#define _PLAYER_H

#include <algorithm>

#include "../VideoEngine/GUI/TextBox.h"
#include "../VideoEngine/GUI/button.h"
#include "../UniverseEngine/universe.h"
#include "../Utils/loader.h"
#include "../Assets/animation.h"
#include "global.h"
#include "ship.h"
#include "shuttle.h"
#include "GravityField.h"
#include "planet.h"

class Player : public Actor, public Object, public InputHandler
{
public:
	Player(
		Common common,
		Shuttle* ship,
		GravityField* gf,
		Planet* planet);
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
	Common _common;
	GravityField* _gf;
	Planet* _planet;

	glm::vec3 _pos;
	glm::vec3 _dirUp;
	glm::vec3 _dirF;
	glm::vec3 _dirR;
	glm::mat4 _matrix;

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
