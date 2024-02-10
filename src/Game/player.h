#ifndef _PLAYER_H
#define _PLAYER_H

#include <algorithm>

#include "../Engine/Video/GUI/TextBox.h"
#include "../Engine/Video/GUI/button.h"
#include "../Engine/Time/TimeEngine.h"
#include "../Engine/Utils/loader.h"
#include "../Engine/Assets/animation.h"
#include "global.h"
#include "shuttle.h"
#include "GravityField.h"
#include "planet.h"

class Sword : public Model
{
public:
	Sword(Common common, Math::Mat<4>* extMat);
	~Sword();

	void Update(float time);

	void Use();

private:
	Common _common;

	Animation _animation;
	float _time;
};

class Player : public Actor, public SoftObject, public InputHandler
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

	bool MouseButton(int button, int action, int mods) override;

	bool InInputArea(float x, float y) override
	{
		return true;
	}

	void Tick() override;
	void TickEarly() override;

	void BuildActions(int key, int action);

private:
	Common _common;
	GravityField* _gf;
	Planet* _planet;

	Math::Vec<3> _pos;
	Math::Vec<3> _dirUp;
	Math::Vec<3> _dirF;
	Math::Vec<3> _dirR;
	Math::Mat<4> _matrix;

	Sword* _sword;

	double _angleH;
	double _angleV;

	int _go;
	int _strafe;
	bool _jump;
	Light _light;
	bool _lightActive;

	Shuttle* _ship;
	bool _buildMode;
	bool _flightMode;
	float _buildCamCoeff;
	Math::Vec<3> _buildCamPos;

	bool _actionERequested;
	bool _actionRRequested;
	bool _actionFRequested;

	TextBox* _centerTextBox;
	TextBox* _cornerTextBox;

	Rectangle _cross;
	uint32_t _crossTexture;
};

#endif
