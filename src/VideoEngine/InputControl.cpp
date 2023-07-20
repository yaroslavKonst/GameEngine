#include "InputControl.h"

InputControl::InputControl(GLFWwindow* window)
{
	_window = window;

	glfwSetWindowUserPointer(_window, this);

	//glfwSetKeyCallback(_window, KeyCallback);
}

InputControl::~InputControl()
{
	glfwSetWindowUserPointer(_window, nullptr);
}
