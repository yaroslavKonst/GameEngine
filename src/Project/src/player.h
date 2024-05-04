#ifndef PLAYER_H
#define PLAYER_H

#include "../../Engine/Time/actor.h"
#include "../../Engine/Video/model.h"
#include "global.h"
#include "GameGlobal.h"

class Player : public Actor, public InputHandler, public Model
{
public:
	Player(
		Engine* engine,
		GameGlobal* gameGlobal,
		std::function<void(double, double)> positionCallback);
	~Player();

	void Tick(double time) override;

	void Key(int key, int scancode, int action, int mods) override;
	bool MouseMoveRaw(double xoffset, double yoffset) override;
	bool Scroll(double xoffset, double yoffset) override;
	bool MouseButton(int button, int action, int mods) override;

private:
	Engine* _engine;
	GameGlobal* _gameGlobal;

	double _x;
	double _y;
	double _z;
	double _angleH;
	double _tAngleH;
	double _roll;

	double _camAngleH;
	double _camAngleV;

	Math::Vec<3> _speed;

	double _go;
	double _strafe;
	bool _sprint;
	int _dash;
	bool _inDash;
	bool _jump;
	Math::Vec<2> _dashDir;

	double _cameraDist;

	Light* _light[4];

	Math::Vec<3> _rLeg;
	Math::Vec<3> _lLeg;
	double _rArm;
	double _lArm;

	std::function<void(double, double)> _positionCallback;

	void SetCameraParams();
	void SetAngleH();

	void ProcessIdle(double surfaceHeight);

	double _legTime;
	double _legStep;
	void ProcessWalk(double surfaceHeight);

	void ProcessRun(double surfaceHeight);

	void ProcessDash(double surfaceHeight);
};

#endif
