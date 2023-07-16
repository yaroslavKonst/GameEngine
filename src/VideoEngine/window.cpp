#include "window.h"

namespace WindowSupport
{
	static int _refCounter = 0;

	void IncRef()
	{
		if (_refCounter == 0) {
			bool success = glfwInit();

			if (!success) {
				throw std::runtime_error(
					"GLFW initialization failed.");
			}
		}

		++_refCounter;
	}

	void DecRef()
	{
		--_refCounter;

		if (_refCounter == 0) {
			glfwTerminate();
		}
	}
}

Window::Window(int width, int height, std::string title)
{
	WindowSupport::IncRef();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	_window = glfwCreateWindow(
		width,
		height,
		title.c_str(),
		nullptr,
		nullptr);

	if (!_window) {
		WindowSupport::DecRef();
		throw std::runtime_error("Failed to create window.");
	}
}

Window::~Window()
{
	glfwDestroyWindow(_window);
	WindowSupport::DecRef();
}
