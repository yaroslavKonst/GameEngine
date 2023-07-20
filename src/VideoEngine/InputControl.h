#ifndef _INPUT_CONTROL_H
#define _INPUT_CONTROL_H

#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class InputHandler
{
public:
	struct HandlerArea
	{
		float x0;
		float y0;
		float x1;
		float y1;
	};

	HandlerArea Area;

	virtual bool InArea(float x, float y)
	{
		return Area.x0 <= x && x <= Area.x1 &&
			Area.y0 <= y && y <= Area.y1;
	}

	InputHandler();
	virtual ~InputHandler();

	virtual bool IsMute()
	{
		return _mute;
	}

	virtual void SetMute(bool mute)
	{
		_mute = mute;
	}

	virtual float GetLayer()
	{
		return _layer;
	}

	virtual void SetLayer(float layer)
	{
		_layer = layer;
	}

	virtual void Key(
		int key,
		int scancode,
		int action,
		int mods)
	{
	}

	virtual bool MouseMove(
		double xpos,
		double ypos,
		bool inArea)
	{
		return false;
	}

	virtual bool MouseButton(
		int button,
		int action,
		int mods)
	{
		return false;
	}

	virtual bool Scroll(
		double xoffset,
		double yoffset)
	{
		return false;
	}

private:
	bool _mute;
	float _layer;
};

class InputControl
{
public:
	InputControl(GLFWwindow* _window);
	~InputControl();

	void Subscribe(InputHandler* handler);
	void UnSubscribe(InputHandler* handler);

	void NotifyWindowClose();

private:
	GLFWwindow* _window;

	float _x;
	float _y;

	std::set<InputHandler*> _handlers;

	static void KeyCallback(
		GLFWwindow* window,
		int key,
		int scancode,
		int action,
		int mods);

	static void CursorPositionCallback(
		GLFWwindow* window,
		double xpos,
		double ypos);

	static void MouseButtonCallback(
		GLFWwindow* window,
		int button,
		int action,
		int mods);

	static void ScrollCallback(
		GLFWwindow* window,
		double xoffset,
		double yoffset);
};

#endif
