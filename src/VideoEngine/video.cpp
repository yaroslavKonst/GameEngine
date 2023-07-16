#include "video.h"

Video::Video(
	int width,
	int height,
	std::string name,
	std::string applicationName)
	: _window(width, height, name)
{
	VkInstanceHandler::SetApplicationName(applicationName);
	VkInstanceHandler::IncRef();

	CreateSurface();
}

Video::~Video()
{
	DestroySurface();

	VkInstanceHandler::DecRef();
}

void Video::CreateSurface()
{
	VkResult res = glfwCreateWindowSurface(
		VkInstanceHandler::GetInstance(),
		_window.GetWindow(),
		nullptr,
		&_surface);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface.");
	}
}

void Video::DestroySurface()
{
	vkDestroySurfaceKHR(
		VkInstanceHandler::GetInstance(),
		_surface,
		nullptr);
}
