#ifndef _TEST_SCENE_H
#define _TEST_SCENE_H

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
#include "../Assets/MovingLight.h"
#include "../Assets/tetraedr.h"
#include "../PhysicalEngine/CollisionEngine.h"

class TestScene
{
public:
	static void UniverseThread(Universe* universe)
	{
		universe->MainLoop();
	}

	static void Run()
	{
		CollisionEngine collisionEngine;

		Tetraedr tetraedr;
		collisionEngine.RegisterObject(&tetraedr);
		Tetraedr tetraedrSt;
		collisionEngine.RegisterObject(&tetraedrSt);

		ExternModel mesh(
			"../src/Assets/Resources/Models/field.obj",
			"../src/Assets/Resources/Models/field.png");
		mesh.SetDrawEnabled(true);
		mesh.SetModelMatrix(
			glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)));

		MovingLight light1(1, 0.01);
		light1.SetLightColor({0.1f, 0.1f, 0.1f});
		light1.SetLightType(Light::Type::Point);

		MovingLight light2(1, -0.01);
		light2.SetLightColor({0.0f, 0.0f, 1.0f});
		light2.SetLightType(Light::Type::Point);

		MovingLight light3(1, 0.1);
		light3.SetLightColor({1.0f, 0.0f, 0.0f});
		light3.SetLightType(Light::Type::Point);

		MovingLight light4(0.1, 0.02);
		light4.SetLightColor({0.0f, 1.0f, 0.0f});
		light4.SetLightType(Light::Type::Spot);
		light4.SetLightAngle(20);
		light4.SetLightAngleFade(5);
		light4.SetLightDirection({0.0f, 0.0f, -1.0f});

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
		universe.RegisterCollisionEngine(&collisionEngine);

		std::thread universeThread(UniverseThread, &universe);

		universe.RegisterActor(&camera);
		universe.RegisterActor(&light1);
		universe.RegisterActor(&light2);
		universe.RegisterActor(&light3);
		universe.RegisterActor(&light4);
		universe.RegisterActor(&tetraedr);

		window.RegisterModel(&mesh);
		window.RegisterModel(&tetraedr);
		window.RegisterModel(&tetraedrSt);
		window.RegisterLight(&light1);
		window.RegisterLight(&light2);
		window.RegisterLight(&light3);
		window.RegisterLight(&light4);

		window.MainLoop();

		window.RemoveModel(&mesh);
		window.RemoveModel(&tetraedr);
		window.RemoveModel(&tetraedrSt);
		window.RemoveLight(&light1);
		window.RemoveLight(&light2);
		window.RemoveLight(&light3);
		window.RemoveLight(&light4);

		universe.RemoveActor(&camera);
		universe.RemoveActor(&light1);
		universe.RemoveActor(&light2);
		universe.RemoveActor(&light3);
		universe.RemoveActor(&light4);
		universe.RemoveActor(&tetraedr);

		universe.Stop();
		universeThread.join();

		universe.RemoveCollisionEngine(&collisionEngine);

		collisionEngine.RemoveObject(&tetraedr);
		collisionEngine.RemoveObject(&tetraedrSt);
	}
};

#endif
