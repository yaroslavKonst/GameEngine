#ifndef _WINDOW_H
#define _WINDOW_H

#include <string>
#include <stdexcept>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window
{
public:
	Window(int width, int height, std::string title);
	~Window();

	GLFWwindow* GetWindow()
	{
		return _window;
	}

private:
	GLFWwindow* _window;
};

#endif
