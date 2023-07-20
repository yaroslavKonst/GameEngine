#include <unistd.h>

#include <thread>

#include "../VideoEngine/video.h"
#include "../UniverseEngine/universe.h"
#include "../Logger/logger.h"
#include "../Assets/triangle.h"
#include "../Assets/square.h"

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
	Square square("../src/Assets/Resources/texture.jpg", 1);
	Square square2("../src/Assets/Resources/transparent.png", -1);

	window.RegisterModel(&triangle);
	universe.RegisterActor(&triangle);

	window.RegisterRectangle(&square);
	universe.RegisterActor(&square);

	window.RegisterRectangle(&square2);
	universe.RegisterActor(&square2);

	window.MainLoop();

	window.RemoveModel(&triangle);
	window.RemoveRectangle(&square);
	window.RemoveRectangle(&square2);
	universe.RemoveActor(&triangle);
	universe.RemoveActor(&square);
	universe.RemoveActor(&square2);

	universe.Stop();
	universeThread.join();

	return 0;
}
