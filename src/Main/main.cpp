#include <unistd.h>

#include <thread>

#include "../VideoEngine/video.h"
#include "../VideoEngine/InputControl.h"
#include "../UniverseEngine/universe.h"
#include "../Logger/logger.h"
#include "../Assets/triangle.h"
#include "../Assets/square.h"
#include "../Assets/loader.h"
#include "../Assets/camera.h"

void UniverseThread(Universe* universe)
{
	universe->MainLoop();
}

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Verbose);

	Video window(800, 600, "Window Title", "Application");

	window.SetFOV(90);
	window.SetCameraPosition(glm::vec3(2.0f, 2.0f, 2.0f));
	window.SetCameraDirection(glm::vec3(-1.0f, -1.0f, -1.0f));
	window.SetCameraUp(glm::vec3(0.0f, 0.0f, 1.0f));

	Camera camera(&window);

	int skTexWidth;
	int skTexHeight;
	auto skTexData = Loader::LoadImage(
		"../src/Assets/Resources/skybox.png",
		skTexWidth,
		skTexHeight);

	skTexWidth /= 6;

	window.CreateSkybox(skTexWidth, skTexHeight, skTexData);

	Universe universe(20);

	std::thread universeThread(UniverseThread, &universe);

	Triangle triangle;
	Square square("../src/Assets/Resources/texture.jpg", 1);
	Square square2("../src/Assets/Resources/transparent.png", -1);

	window.GetInputControl()->Subscribe(&square);

	window.RegisterModel(&triangle);
	universe.RegisterActor(&triangle);

	window.RegisterRectangle(&square);
	universe.RegisterActor(&square);

	window.RegisterRectangle(&square2);
	universe.RegisterActor(&square2);

	universe.RegisterActor(&camera);

	window.MainLoop();

	window.RemoveModel(&triangle);
	window.RemoveRectangle(&square);
	window.RemoveRectangle(&square2);
	universe.RemoveActor(&triangle);
	universe.RemoveActor(&square);
	universe.RemoveActor(&square2);

	universe.RemoveActor(&camera);

	universe.Stop();
	universeThread.join();

	return 0;
}
