#include "InputControl.h"

#include <map>

InputControl::InputControl(GLFWwindow* window)
{
	_window = window;
	_x = 0;
	_y = 0;
	_rawX = 0;
	_rawY = 0;
	_rawMouseInput = false;
	_requestedRawMouseInput = false;
	_continuousRawInput = false;

	glfwSetWindowUserPointer(_window, this);

	glfwSetWindowCloseCallback(_window, WindowCloseCallback);
	glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
	glfwSetKeyCallback(_window, KeyCallback);
	glfwSetCursorPosCallback(_window, CursorPositionCallback);
	glfwSetMouseButtonCallback(_window, MouseButtonCallback);
	glfwSetScrollCallback(_window, ScrollCallback);
}

InputControl::~InputControl()
{
	glfwSetWindowUserPointer(_window, nullptr);

	glfwSetWindowCloseCallback(_window, nullptr);
	glfwSetFramebufferSizeCallback(_window, nullptr);
	glfwSetKeyCallback(_window, nullptr);
	glfwSetCursorPosCallback(_window, nullptr);
	glfwSetMouseButtonCallback(_window, nullptr);
	glfwSetScrollCallback(_window, nullptr);
}

void InputControl::Subscribe(InputHandler* handler)
{
	_mutex.Lock();
	_handlers.insert(handler);
	_mutex.Unlock();
}

void InputControl::Unsubscribe(InputHandler* handler)
{
	_mutex.Lock();
	_handlers.erase(handler);
	_mutex.Unlock();
}

void InputControl::ToggleRawMouseInput()
{
	_requestedRawMouseInput = !_requestedRawMouseInput;
}

void InputControl::ToggleRawMouseInputInternal()
{
	if (_rawMouseInput == _requestedRawMouseInput) {
		return;
	}

	if (!_rawMouseInput) {
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		_rawMouseInput = true;
	} else {
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		_rawMouseInput = false;
		_continuousRawInput = false;
	}
}

void InputControl::PollEvents()
{
	ToggleRawMouseInputInternal();
	glfwPollEvents();
}

void InputControl::FramebufferResizeCallback(
	GLFWwindow* window,
	int width,
	int height)
{
	InputControl* control = reinterpret_cast<InputControl*>(
		glfwGetWindowUserPointer(window));

	std::set<InputHandler*> activeHandlers;

	for (auto handler : control->_handlers) {
		if (handler->IsInputEnabled()) {
			activeHandlers.insert(handler);
		}
	}

	for (auto handler : activeHandlers) {
		handler->WindowResize();
	}
}

void InputControl::WindowCloseCallback(GLFWwindow* window)
{
	InputControl* control = reinterpret_cast<InputControl*>(
		glfwGetWindowUserPointer(window));

	glfwSetWindowShouldClose(window, GLFW_FALSE);

	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : control->_handlers) {
		if (!handler->IsInputEnabled()) {
			continue;
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed =
			handler.second->WindowClose();

		if (processed) {
			break;
		}
	}
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

	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : control->_handlers) {
		if (handler->IsInputEnabled()) {
			orderedHandlers[handler->GetInputLayer()] = handler;
		}
	}

	for (auto handler : orderedHandlers) {
		bool processed = handler.second->Key(
			key,
			scancode,
			action,
			mods);

		if (processed) {
			break;
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

	if (control->_rawMouseInput) {
		RawCursorPositionCallback(control, xpos, ypos);
		return;
	}

	control->_continuousRawInput = false;

	int width;
	int height;
	glfwGetWindowSize(window, &width, &height);

	float x = ((xpos / width) * 2 - 1) * width / height;
	float y = (ypos / height) * 2 - 1;

	control->_rawX = xpos;
	control->_rawY = ypos;

	control->_x = x;
	control->_y = y;

	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : control->_handlers) {
		if (!handler->IsInputEnabled()) {
			continue;
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	bool processed = false;

	for (auto handler : orderedHandlers) {
		if (!handler.second->InInputArea(x, y)) {
			handler.second->MouseMove(0, 0, false);
			continue;
		}

		if (!processed) {
			float locX = x - handler.second->InputArea.x0;
			float locY = y - handler.second->InputArea.y0;

			bool proc = handler.second->MouseMove(locX, locY, true);

			if (proc) {
				processed = true;
			}
		} else {
			handler.second->MouseMove(0, 0, false);
		}
	}
}

void InputControl::RawCursorPositionCallback(
	InputControl* control,
	double xpos,
	double ypos)
{
	float xoffset = xpos - control->_rawX;
	float yoffset = ypos - control->_rawY;

	if (!control->_continuousRawInput) {
		control->_continuousRawInput = true;
		xoffset = 0;
		yoffset = 0;
	}

	control->_rawX = xpos;
	control->_rawY = ypos;

	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : control->_handlers) {
		if (!handler->IsInputEnabled()) {
			continue;
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed = handler.second->MouseMoveRaw(
			xoffset,
			yoffset);

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
		if (!handler->IsInputEnabled()) {
			continue;
		}

		if (!control->_rawMouseInput) {
			if (!handler->InInputArea(x, y)) {
				continue;
			}
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed =
			handler.second->MouseButton(
				button,
				action,
				mods);

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
		if (!handler->IsInputEnabled()) {
			continue;
		}

		if (!control->_rawMouseInput) {
			if (!handler->InInputArea(x, y)) {
				continue;
			}
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed = handler.second->Scroll(
			xoffset,
			yoffset);

		if (processed) {
			break;
		}
	}
}

// Handler methods.
InputHandler::InputHandler()
{
	_inputEnabled = false;
	InputArea.x0 = 0;
	InputArea.y0 = 0;
	InputArea.x1 = 0;
	InputArea.y1 = 0;
	_layer = 0;
}

InputHandler::~InputHandler()
{
}
