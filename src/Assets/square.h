#ifndef _SQUARE_H
#define _SQUARE_H

#include "../VideoEngine/rectangle.h"
#include "../VideoEngine/InputControl.h"
#include "../UniverseEngine/actor.h"
#include "../Logger/logger.h"

class Square : public Rectangle, public Actor, public InputHandler
{
public:
	Square(const char* texturePath, float depthMod);
	~Square();
	void Tick();
	bool MouseMove(double xpos, double ypos, bool inArea)
	{
		Logger::Verbose() << inArea << " " << xpos << " " << ypos;
		return true;
	}

	void Key(int key, int scancode, int action, int mods)
	{
		Logger::Verbose() << key << " " << scancode << " " <<
			action << " " << mods;
	}

	bool MouseButton(int button, int action, int mods)
	{
		Logger::Verbose() << button << " " <<
			action << " " << mods;
		return true;
	}

	bool Scroll(double xoffset, double yoffset)
	{
		Logger::Verbose() << xoffset << " " << yoffset;
		return true;
	}

private:
	float _time;
	float _depthMod;
};

#endif