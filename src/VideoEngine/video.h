#ifndef _VIDEO_H
#define _VIDEO_H

#include "window.h"
#include "VkInstanceHandler.h"

class Video
{
public:
	Video(
		int width,
		int height,
		std::string name,
		std::string applicationName = "");

	~Video();

private:
	int _width;
	int _height;

	Window _window;

	VkSurfaceKHR _surface;
	void CreateSurface();
	void DestroySurface();
};

#endif
