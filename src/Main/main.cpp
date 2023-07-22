#include <unistd.h>

#include <thread>

#include "../VideoEngine/video.h"
#include "../VideoEngine/InputControl.h"
#include "../UniverseEngine/universe.h"
#include "../Logger/logger.h"
#include "../Assets/triangle.h"
#include "../Assets/square.h"
#include "../Utils/loader.h"
#include "../Assets/camera.h"
#include "../Assets/ExternModel.h"

void UniverseThread(Universe* universe)
{
	universe->MainLoop();
}

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Verbose);

	ExternModel mesh(
		"../src/Assets/Resources/Models/field.obj",
		"../src/Assets/Resources/Models/field.png");
	mesh.SetDrawEnabled(true);
	mesh.SetModelMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)));

	Video window(1400, 1000, "Video", "Application");

	window.SetFOV(60);
	window.SetCameraPosition(glm::vec3(2.0f, 0.0f, 2.0f));
	window.SetCameraDirection(glm::vec3(-1.0f, -1.0f, -1.0f));
	window.SetCameraUp(glm::vec3(0.0f, 0.0f, 1.0f));

	Camera camera(&window);

	int skTexWidth;
	int skTexHeight;
	auto skTexData = Loader::LoadImage(
		"../src/Assets/Resources/Skybox/skybox.png",
		skTexWidth,
		skTexHeight);

	window.CreateSkybox(skTexWidth, skTexHeight, skTexData);

	Universe universe(20);

	std::thread universeThread(UniverseThread, &universe);

	universe.RegisterActor(&camera);
	window.RegisterModel(&mesh);

	window.MainLoop();

	window.RemoveModel(&mesh);
	universe.RemoveActor(&camera);

	universe.Stop();
	universeThread.join();

	return 0;
}
