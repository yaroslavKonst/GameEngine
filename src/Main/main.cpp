#include <unistd.h>

#include <thread>

#include "../VideoEngine/video.h"
#include "../UniverseEngine/universe.h"
#include "../Logger/logger.h"
#include "../Assets/triangle.h"

void UniverseThread(Universe* universe)
{
	universe->MainLoop();
}

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Verbose);

	Video window(800, 600, "Window Title", "Application");
	Universe universe(20);

	std::thread universeThread(UniverseThread, &universe);

	Triangle triangle;
	window.RegisterModel(&triangle);

	window.MainLoop();

	window.RemoveModel(&triangle);

	universe.Stop();
	universeThread.join();

	return 0;
}
