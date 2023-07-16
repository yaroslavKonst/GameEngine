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
}

Video::~Video()
{
	VkInstanceHandler::DecRef();
}
