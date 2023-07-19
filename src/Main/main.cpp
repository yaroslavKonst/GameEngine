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

	window.SetViewMatrix(glm::lookAt(
		glm::vec3(2.0f, 2.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)));

	window.SetFOV(45);

	Universe universe(20);

	std::thread universeThread(UniverseThread, &universe);

	Triangle triangle;
	window.RegisterModel(&triangle);
	universe.RegisterActor(&triangle);

	window.MainLoop();

	window.RemoveModel(&triangle);
	universe.RemoveActor(&triangle);

	universe.Stop();
	universeThread.join();

	return 0;
}
