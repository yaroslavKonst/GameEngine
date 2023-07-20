#ifndef _INPUT_CONTROL_H
#define _INPUT_CONTROL_H

#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class InputControl
{
public:
	struct Area
	{
		float x0;
		float y0;
		float x1;
		float y1;
	};

	typedef void (*KeyCallbackType)(
		void* pointer,
		int key,
		int scancode,
		int action,
		int mods);

	typedef bool (*MouseMoveCallbackType)(
		void* pointer,
		double xpos,
		double ypos);

	typedef bool (*MouseButtonCallbackType)(
		void* pointer,
		int button,
		int action,
		int mods);

	typedef bool (*ScrollCallbackType)(
		void* pointer,
		double xoffset,
		double yoffset);

	typedef void (*WindowCloseCallbackType)(void* pointer);

	InputControl(GLFWwindow* _window);
	~InputControl();

	void SubscribeKeyEvent(KeyCallbackType callback);
	void SubscribeMouseMoveEvent(
		Area* area,
		MouseMoveCallbackType callback);
	void SubscribeMouseButtonEvent(
		Area* area,
		MouseButtonCallbackType callback);
	void SubscribeScrollEvent(Area* area, ScrollCallbackType callback);
	void SubscribeWindowCloseEvent(WindowCloseCallbackType callback);

	void NotifyWindowClose();

private:
	GLFWwindow* _window;



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
