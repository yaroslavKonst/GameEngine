#include "InputControl.h"

#include <map>

InputControl::InputControl(GLFWwindow* window)
{
	_window = window;
	_x = 0;
	_y = 0;

	glfwSetWindowUserPointer(_window, this);

	glfwSetKeyCallback(_window, KeyCallback);
	glfwSetCursorPosCallback(_window, CursorPositionCallback);
	glfwSetMouseButtonCallback(_window, MouseButtonCallback);
	glfwSetScrollCallback(_window, ScrollCallback);
}

InputControl::~InputControl()
{
	glfwSetWindowUserPointer(_window, nullptr);

	glfwSetKeyCallback(_window, nullptr);
	glfwSetCursorPosCallback(_window, nullptr);
	glfwSetMouseButtonCallback(_window, nullptr);
	glfwSetScrollCallback(_window, nullptr);
}

void InputControl::Subscribe(InputHandler* handler)
{
	_handlers.insert(handler);
}

void InputControl::UnSubscribe(InputHandler* handler)
{
	_handlers.erase(handler);
}

void InputControl::KeyCallback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mods)
{
	InputControl* control = reinterpret_cast<InputControl*>(
		glfwGetWindowUserPointer(window));

	for (auto handler : control->_handlers) {
		if (!handler->IsMute()) {
			handler->Key(key, scancode, action, mods);
		}
	}
}

void InputControl::CursorPositionCallback(
	GLFWwindow* window,
	double xpos,
	double ypos)
{
	InputControl* control = reinterpret_cast<InputControl*>(
		glfwGetWindowUserPointer(window));

	int width;
	int height;
	glfwGetWindowSize(window, &width, &height);

	float x = (xpos / width) * 2 - 1;
	float y = (ypos / height) * 2 - 1;

	control->_x = x;
	control->_y = y;

	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : control->_handlers) {
		if (handler->IsMute()) {
			continue;
		}

		if (!handler->InArea(x, y)) {
			handler->MouseMove(0, 0, false);
			continue;
		}

		orderedHandlers[handler->GetLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		float locX = (x - handler.second->Area.x0) /
			(handler.second->Area.x1 - handler.second->Area.x0);
		float locY = (y - handler.second->Area.y0) /
			(handler.second->Area.y1 - handler.second->Area.y0);

		bool processed = handler.second->MouseMove(locX, locY, true);

		if (processed) {
			break;
		}
	}
}

void InputControl::MouseButtonCallback(
	GLFWwindow* window,
	int button,
	int action,
	int mods)
{
	InputControl* control = reinterpret_cast<InputControl*>(
		glfwGetWindowUserPointer(window));

	float x = control->_x;
	float y = control->_y;

	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : control->_handlers) {
		if (handler->IsMute()) {
			continue;
		}

		if (!handler->InArea(x, y)) {
			continue;
		}

		orderedHandlers[handler->GetLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed =
			handler.second->MouseButton(button, action, mods);

		if (processed) {
			break;
		}
	}
}

void InputControl::ScrollCallback(
	GLFWwindow* window,
	double xoffset,
	double yoffset)
{
	InputControl* control = reinterpret_cast<InputControl*>(
		glfwGetWindowUserPointer(window));

	float x = control->_x;
	float y = control->_y;

	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : control->_handlers) {
		if (handler->IsMute()) {
			continue;
		}

		if (!handler->InArea(x, y)) {
			continue;
		}

		orderedHandlers[handler->GetLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed = handler.second->Scroll(xoffset, yoffset);

		if (processed) {
			break;
		}
	}

}

// Handler methods.
InputHandler::InputHandler()
{
	_mute = true;
}

InputHandler::~InputHandler()
{
}
