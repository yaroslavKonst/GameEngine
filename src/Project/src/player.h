#ifndef PLAYER_H
#define PLAYER_H

#include "../../Engine/Time/actor.h"
#include "../../Engine/Video/model.h"
#include "global.h"
#include "GameGlobal.h"

class Player : public Actor, public InputHandler, public Model
{
public:
	Player(Engine* engine, GameGlobal* gameGlobal);
	~Player();

	void Tick(double time) override;

	void Key(int key, int scancode, int action, int mods) override;
	bool MouseMoveRaw(double xoffset, double yoffset) override;
	bool Scroll(double xoffset, double yoffset) override;

private:
	Engine* _engine;
	GameGlobal* _gameGlobal;

	double _x;
	double _y;
	double _angleH;
	double _angleV;

	double _go;
	double _strafe;

	double _cameraDist;

	Light* _light[4];

	double _legTime;
	double _legStep;

	double _debugAngle;
};

#endif
