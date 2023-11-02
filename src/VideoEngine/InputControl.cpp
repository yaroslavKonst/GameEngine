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

void InputControl::ToggleRawMouseInput()
{
	if (!_rawMouseInput) {
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		_rawMouseInput = true;
	} else {
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		_rawMouseInput = false;
	}
}

void InputControl::PollEvents()
{
	_submittedEvents.KeyEvents.resize(_polledEvents.KeyEvents.size());
	_submittedEvents.CursorPositionEvents.resize(
		_polledEvents.CursorPositionEvents.size());
	_submittedEvents.RawCursorPositionEvents.resize(
		_polledEvents.RawCursorPositionEvents.size());
	_submittedEvents.MouseButtonEvents.resize(
		_polledEvents.MouseButtonEvents.size());
	_submittedEvents.ScrollEvents.resize(_polledEvents.ScrollEvents.size());

	size_t idx = 0;

	for (auto& event : _polledEvents.KeyEvents) {
		_submittedEvents.KeyEvents[idx] = event;
		++idx;
	}

	idx = 0;

	for (auto& event : _polledEvents.CursorPositionEvents) {
		_submittedEvents.CursorPositionEvents[idx] = event;
		++idx;
	}

	idx = 0;

	for (auto& event : _polledEvents.RawCursorPositionEvents) {
		_submittedEvents.RawCursorPositionEvents[idx] = event;
		++idx;
	}

	idx = 0;

	for (auto& event : _polledEvents.MouseButtonEvents) {
		_submittedEvents.MouseButtonEvents[idx] = event;
		++idx;
	}

	idx = 0;

	for (auto& event : _polledEvents.ScrollEvents) {
		_submittedEvents.ScrollEvents[idx] = event;
		++idx;
	}

	_polledEvents.KeyEvents.clear();
	_polledEvents.CursorPositionEvents.clear();
	_polledEvents.RawCursorPositionEvents.clear();
	_polledEvents.MouseButtonEvents.clear();
	_polledEvents.ScrollEvents.clear();
}

void InputControl::SubmitEvents()
{
	_pendingEvents = _submittedEvents;

	_submittedEvents.KeyEvents.clear();
	_submittedEvents.CursorPositionEvents.clear();
	_submittedEvents.RawCursorPositionEvents.clear();
	_submittedEvents.MouseButtonEvents.clear();
	_submittedEvents.ScrollEvents.clear();
}

void InputControl::InvokeEvents()
{
	for (auto& event : _pendingEvents.KeyEvents) {
		KeyProcess(event);
	}

	_pendingEvents.KeyEvents.clear();

	for (auto& event : _pendingEvents.CursorPositionEvents) {
		CursorPositionProcess(event);
	}

	_pendingEvents.CursorPositionEvents.clear();

	for (auto& event : _pendingEvents.RawCursorPositionEvents) {
		RawCursorPositionProcess(event);
	}

	_pendingEvents.RawCursorPositionEvents.clear();

	for (auto& event : _pendingEvents.MouseButtonEvents) {
		MouseButtonProcess(event);
	}

	_pendingEvents.MouseButtonEvents.clear();

	for (auto& event : _pendingEvents.ScrollEvents) {
		ScrollProcess(event);
	}

	_pendingEvents.ScrollEvents.clear();
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

	KeyData event;
	event.Key = key;
	event.Scancode = scancode;
	event.Action = action;
	event.Mods = mods;

	control->_polledEvents.KeyEvents.push_back(event);
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

	int width;
	int height;
	glfwGetWindowSize(window, &width, &height);

	float x = (xpos / width) * 2 - 1;
	float y = (ypos / height) * 2 - 1;

	control->_rawX = xpos;
	control->_rawY = ypos;

	control->_x = x;
	control->_y = y;

	CursorPositionData event;
	event.XPos = xpos;
	event.YPos = ypos;
	event.x = x;
	event.y = y;

	control->_polledEvents.CursorPositionEvents.push_back(event);
}

void InputControl::RawCursorPositionCallback(
	InputControl* control,
	double xpos,
	double ypos)
{
	float xoffset = xpos - control->_rawX;
	float yoffset = ypos - control->_rawY;

	control->_rawX = xpos;
	control->_rawY = ypos;

	CursorPositionData event;
	event.XPos = xoffset;
	event.YPos = yoffset;

	control->_polledEvents.RawCursorPositionEvents.push_back(event);
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

	MouseButtonData event;
	event.Button = button;
	event.Action = action;
	event.Mods = mods;
	event.x = x;
	event.y = y;

	control->_polledEvents.MouseButtonEvents.push_back(event);
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

	ScrollData event;
	event.XOffset = xoffset;
	event.YOffset = yoffset;
	event.x = x;
	event.y = y;

	control->_polledEvents.ScrollEvents.push_back(event);
}

void InputControl::KeyProcess(KeyData& event)
{
	std::set<InputHandler*> activeHandlers;

	for (auto handler : _handlers) {
		if (handler->IsInputEnabled()) {
			activeHandlers.insert(handler);
		}
	}

	for (auto handler : activeHandlers) {
		handler->Key(
			event.Key,
			event.Scancode,
			event.Action,
			event.Mods);
	}
}

void InputControl::CursorPositionProcess(CursorPositionData& event)
{
	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : _handlers) {
		if (!handler->IsInputEnabled()) {
			continue;
		}

		if (!handler->InInputArea(event.x, event.y)) {
			handler->MouseMove(0, 0, false);
			continue;
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		float locX = (event.x - handler.second->InputArea.x0) /
			(handler.second->InputArea.x1 -
			handler.second->InputArea.x0);
		float locY = (event.y - handler.second->InputArea.y0) /
			(handler.second->InputArea.y1 -
			handler.second->InputArea.y0);

		bool processed = handler.second->MouseMove(locX, locY, true);

		if (processed) {
			break;
		}
	}
}

void InputControl::RawCursorPositionProcess(CursorPositionData& event)
{
	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : _handlers) {
		if (!handler->IsInputEnabled()) {
			continue;
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed = handler.second->MouseMoveRaw(
			event.XPos,
			event.YPos);

		if (processed) {
			break;
		}
	}
}

void InputControl::MouseButtonProcess(MouseButtonData& event)
{
	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : _handlers) {
		if (!handler->IsInputEnabled()) {
			continue;
		}

		if (!handler->InInputArea(event.x, event.y)) {
			continue;
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed =
			handler.second->MouseButton(
				event.Button,
				event.Action,
				event.Mods);

		if (processed) {
			break;
		}
	}
}

void InputControl::ScrollProcess(ScrollData& event)
{
	std::map<float, InputHandler*> orderedHandlers;

	for (auto handler : _handlers) {
		if (!handler->IsInputEnabled()) {
			continue;
		}

		if (!handler->InInputArea(event.x, event.y)) {
			continue;
		}

		orderedHandlers[handler->GetInputLayer()] = handler;
	}

	for (auto handler : orderedHandlers) {
		bool processed = handler.second->Scroll(
			event.XOffset,
			event.YOffset);

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
