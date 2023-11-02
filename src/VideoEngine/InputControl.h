#ifndef _INPUT_CONTROL_H
#define _INPUT_CONTROL_H

#include <set>
#include <list>
#include <vector>

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

	HandlerArea InputArea;

	virtual bool InInputArea(float x, float y)
	{
		return InputArea.x0 <= x && x <= InputArea.x1 &&
			InputArea.y0 <= y && y <= InputArea.y1;
	}

	InputHandler();
	virtual ~InputHandler();

	virtual bool IsInputEnabled()
	{
		return _inputEnabled;
	}

	virtual void SetInputEnabled(bool enabled)
	{
		_inputEnabled = enabled;
	}

	virtual float GetInputLayer()
	{
		return _layer;
	}

	virtual void SetInputLayer(float layer)
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

	virtual bool MouseMoveRaw(
		double xoffset,
		double yoffset)
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
	bool _inputEnabled;
	float _layer;
};

class InputControl
{
public:
	InputControl(GLFWwindow* _window);
	~InputControl();

	void Subscribe(InputHandler* handler);
	void UnSubscribe(InputHandler* handler);

	void PollEvents();
	void SubmitEvents();
	void InvokeEvents();

	void ToggleRawMouseInput();

	void NotifyWindowClose();

private:
	struct KeyData
	{
		int Key;
		int Scancode;
		int Action;
		int Mods;
	};

	struct CursorPositionData
	{
		double XPos;
		double YPos;
		float x;
		float y;
	};

	struct MouseButtonData
	{
		int Button;
		int Action;
		int Mods;
		float x;
		float y;
	};

	struct ScrollData
	{
		double XOffset;
		double YOffset;
		float x;
		float y;
	};

	struct PolledEventData
	{
		std::list<KeyData> KeyEvents;
		std::list<CursorPositionData> CursorPositionEvents;
		std::list<CursorPositionData> RawCursorPositionEvents;
		std::list<MouseButtonData> MouseButtonEvents;
		std::list<ScrollData> ScrollEvents;
	};

	struct EventData
	{
		std::vector<KeyData> KeyEvents;
		std::vector<CursorPositionData> CursorPositionEvents;
		std::vector<CursorPositionData> RawCursorPositionEvents;
		std::vector<MouseButtonData> MouseButtonEvents;
		std::vector<ScrollData> ScrollEvents;
	};

	PolledEventData _polledEvents;
	EventData _submittedEvents;
	EventData _pendingEvents;

	GLFWwindow* _window;

	float _x;
	float _y;

	float _rawX;
	float _rawY;

	bool _rawMouseInput;

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

	static void RawCursorPositionCallback(
		InputControl* control,
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

	void KeyProcess(KeyData& event);
	void CursorPositionProcess(CursorPositionData& event);
	void RawCursorPositionProcess(CursorPositionData& event);
	void MouseButtonProcess(MouseButtonData& event);
	void ScrollProcess(ScrollData& event);
};

#endif
