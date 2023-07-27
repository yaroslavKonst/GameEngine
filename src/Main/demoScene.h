#ifndef _DEMO_H
#define _DEMO_H

#include <algorithm>

#include "../VideoEngine/video.h"
#include "../UniverseEngine/universe.h"
#include "../Utils/loader.h"
#include "../Assets/square.h"
#include "../Assets/ExternModel.h"

class Player : public Actor, public Object, public InputHandler
{
public:
	Player(Video* video, Light* light)
	{
		_video = video;
		_pos = glm::vec3(0.0, 0.0, 5.0);
		_angleH = 0;
		_angleV = 0;
		_go = 0;
		_strafe = 0;
		_jump = false;
		_vspeed = 0;
		_light = light;
		_lightActive = true;

		// Pyramid
		std::vector<glm::vec3> vertices;
		vertices.push_back(glm::vec3(0.1, -0.1, 0.0));
		vertices.push_back(glm::vec3(0.1, 0.1, 0.0));
		vertices.push_back(glm::vec3(-0.1, 0.0, 0.0));
		vertices.push_back(glm::vec3(0.1, -0.1, 1.0));
		vertices.push_back(glm::vec3(0.1, 0.1, 1.0));
		vertices.push_back(glm::vec3(-0.1, 0.0, 1.0));
		vertices.push_back(glm::vec3(0.0, 0.0, 2.0));

		std::vector<uint32_t> indices = {0, 1, 2};

		SetObjectVertices(vertices);
		SetObjectIndices(indices);
		SetObjectCenter({0.0f, 0.0f, 1.5f});
		SetObjectMatrix(glm::mat4(1.0));

		SetInputEnabled(true);
	}

	void Key(
		int key,
		int scancode,
		int action,
		int mods)
	{
		if (key == GLFW_KEY_C) {
			if (action == GLFW_PRESS) {
				_mutex.lock();
				_video->GetInputControl()->
					ToggleRawMouseInput();
				_mutex.unlock();
			}
		} else if (key == GLFW_KEY_W) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_go += 1;
			} else if (action == GLFW_RELEASE) {
				_go -= 1;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_S) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_go -= 1;
			} else if (action == GLFW_RELEASE) {
				_go += 1;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_D) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_strafe += 1;
			} else if (action == GLFW_RELEASE) {
				_strafe -= 1;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_A) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_strafe -= 1;
			} else if (action == GLFW_RELEASE) {
				_strafe += 1;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_SPACE) {
			_mutex.lock();
			if (action == GLFW_PRESS) {
				_jump = true;
			} else if (action == GLFW_RELEASE) {
				_jump = false;
			}
			_mutex.unlock();
		} else if (key == GLFW_KEY_L) {
			if (action == GLFW_PRESS) {
				if (_lightActive) {
					_video->RemoveLight(_light);
					_lightActive = false;
				} else {
					_video->RegisterLight(_light);
					_lightActive = true;
				}
			}
		}
	}

	bool MouseMoveRaw(
		double xoffset,
		double yoffset)
	{
		_mutex.lock();
		_angleH += xoffset * 0.1;
		_angleV += yoffset * 0.1;

		if (_angleH < 0) {
			_angleH += 360;
		} else if (_angleH >= 360) {
			_angleH -= 360;
		}

		_angleV = std::clamp(_angleV, -85.0f, 85.0f);
		_mutex.unlock();

		return true;
	}

	void Tick()
	{
		_mutex.lock();
		glm::vec2 hdir(
			sinf(glm::radians(_angleH)),
			cosf(glm::radians(_angleH)));

		glm::vec2 hdirStrafe(
			sinf(glm::radians(_angleH + 90)),
			cosf(glm::radians(_angleH + 90)));

		glm::vec2 hspeed = hdir * (float)_go +
			hdirStrafe * (float)_strafe;

		_vspeed -= 0.098 / 2;

		glm::vec3 effect = GetObjectEffect();

		if (effect.z > 0 && _vspeed < 0) {
			if (fabs(_vspeed) > 0.0001) {
				_vspeed = -_vspeed * 0.4;
			} else {
				if (effect.z < 0.0000001) {
					_vspeed = 0;
				} else {
					_vspeed = 0.1;
				}
			}
		}

		if (effect.z > 0 && _jump) {
			_vspeed = 6;
		}

		if (fabs(effect.x) + fabs(effect.y) > 0.0001) {
			if (effect.x * hspeed.x + effect.y * hspeed.y < 0) {
				hspeed = glm::vec2(0, 0);
			}
		}

		_pos.x += hspeed.x / 20;
		_pos.y += hspeed.y / 20;
		_pos.z += _vspeed / 200;

		SetObjectMatrix(
			glm::rotate(glm::translate(glm::mat4(1.0), _pos),
				glm::radians(_angleH), glm::vec3(0, 0, 1)));
		SetObjectSpeed(glm::vec3(
			hspeed.x / 20,
			hspeed.y / 20,
			_vspeed / 200));

		_video->SetCameraPosition(_pos + glm::vec3(0, 0, 1.85));
		_video->SetCameraDirection(glm::vec3(
				hdir * cosf(glm::radians(_angleV)),
				sinf(glm::radians(_angleV))));

		_light->SetLightPosition(_pos + glm::vec3(0, 0, 1.2) +
				glm::vec3(hdirStrafe * 0.3f, 0.0f));
		_light->SetLightDirection(glm::vec3(
				hdir * cosf(glm::radians(_angleV)),
				sinf(glm::radians(_angleV))));
		_mutex.unlock();
	}

private:
	Video* _video;
	glm::vec3 _pos;
	float _angleH;
	float _angleV;
	float _vspeed;

	int _go;
	int _strafe;
	bool _jump;
	Light* _light;
	bool _lightActive;

	std::mutex _mutex;
};

class Field : public Model, public Object
{
public:
	Field()
	{
		std::vector<glm::vec3> objectVertices = {
			{-200, -200, 0.0},
			{-200, 200, 0.0},
			{200, 200, 0.0},
			{200, -200, 0.0},
		};

		std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

		SetObjectVertices(objectVertices);
		SetObjectIndices(indices);
		SetObjectCenter({0.0f, 0.0f, -300.0f});
		SetObjectMatrix(glm::mat4(1.0));
		SetModelMatrix(
			glm::translate(
			glm::scale(
				glm::mat4(1.0),
				glm::vec3(10, 10, 1)),
				glm::vec3(0, 0, -0.251981)));
		SetModelInnerMatrix(glm::mat4(1.0));
		SetModelInstances({glm::mat4(1.0)});

		int texWidth;
		int texHeight;
		auto texture = Loader::LoadImage(
			"../src/Assets/Resources/Models/floor.jpg",
			texWidth,
			texHeight);
		auto model = Loader::LoadModel(
			"../src/Assets/Resources/Models/field.obj");

		for (auto& coord : model.TexCoords) {
			coord *= 100;
		}

		SetModelVertices(model.Vertices);
		SetModelNormals(model.Normals);
		SetModelTexCoords(model.TexCoords);
		SetModelIndices(model.Indices);

		SetTexWidth(texWidth);
		SetTexHeight(texHeight);
		SetTexData(texture);

		SetDrawEnabled(true);
	}
};

class Demo
{
public:
	static void Run()
	{
		std::mutex sceneMutex;

		CollisionEngine collisionEngine;
		// 5 ms between universe ticks.
		Universe universe(5);

		Video::GraphicsSettings videoSettings{};
		videoSettings.MsaaLimit = 2;

		Video video(1400, 1000, "Demo", "Application", &videoSettings);

		video.SetSceneMutex(&sceneMutex);
		universe.SetSceneMutex(&sceneMutex);

		universe.RegisterCollisionEngine(&collisionEngine);

		Light light;
		light.SetLightType(Light::Type::Spot);
		light.SetLightColor({0.9, 0.9, 0.9});
		light.SetLightAngle(15);
		light.SetLightAngleFade(10);

		video.SetFOV(80);
		video.SetCameraUp({0, 0, 1});
		int skbTWidth;
		int skbTHeight;
		auto skbTData = Loader::LoadImage(
			"../src/Assets/Resources/Skybox/skybox.png",
			skbTWidth,
			skbTHeight);
		video.CreateSkybox(skbTWidth, skbTHeight, skbTData);
		video.SetSkyboxColor({0.1, 0.05, 0.05});

		Player player(&video, &light);
		Field field;

		glm::mat4 base = glm::translate(
			glm::mat4(1.0),
			glm::vec3(20, 0, 0));

		ExternModel wall1(
			"../src/Assets/Resources/Models/wall.obj",
			"../src/Assets/Resources/Models/wall.png",
			base);

		base = glm::translate(base, glm::vec3(11, 0, 0));

		ExternModel wall2(
			"../src/Assets/Resources/Models/wall.obj",
			"../src/Assets/Resources/Models/wall.png",
			base);

		base = glm::rotate(
			base,
			glm::radians(90.0f),
			glm::vec3(0, 0, 1));

		ExternModel wall3(
			"../src/Assets/Resources/Models/wall.obj",
			"../src/Assets/Resources/Models/wall.png",
			base);

		base = glm::translate(base, glm::vec3(10, 3, 0));

		ExternModel wall4(
			"../src/Assets/Resources/Models/wall.obj",
			"../src/Assets/Resources/Models/wall.png",
			base);

		ExternModel wallLight(
			"../src/Assets/Resources/Models/wall.obj",
			"../src/Assets/Resources/Models/wall.png",
			glm::scale(
				glm::translate(
					glm::mat4(1.0f),
				glm::vec3(25, 5, 2.5)),
					glm::vec3(0.1, 0.1, 0.1)));

		ExternModel roof(
			"../src/Assets/Resources/Models/roof.obj",
			"../src/Assets/Resources/Models/wall.png",
			glm::scale(
				glm::translate(
					glm::mat4(1.0f),
				glm::vec3(21, 12, 3)),
					glm::vec3(1, 1, 1)));

		wallLight.SetDrawLight(true);
		wallLight.SetDrawLightMultiplier(10);

		Light lightSt;
		lightSt.SetLightType(Light::Type::Point);
		lightSt.SetLightColor({20.0, 20.0, 20.0});
		lightSt.SetLightPosition({25, 5.5, 2.5});

		Square square("../src/Assets/Resources/Images/texture.jpg", 0);

		universe.RegisterActor(&player);

		collisionEngine.RegisterObject(&player);
		collisionEngine.RegisterObject(&field);

		collisionEngine.RegisterObject(&wallLight);
		collisionEngine.RegisterObject(&wall1);
		collisionEngine.RegisterObject(&wall2);
		collisionEngine.RegisterObject(&wall3);
		collisionEngine.RegisterObject(&wall4);
		collisionEngine.RegisterObject(&roof);

		video.RegisterModel(&field);

		video.RegisterModel(&wallLight);
		video.RegisterModel(&wall1);
		video.RegisterModel(&wall2);
		video.RegisterModel(&wall3);
		video.RegisterModel(&wall4);
		video.RegisterModel(&roof);

		video.RegisterRectangle(&square);

		video.RegisterLight(&light);
		video.RegisterLight(&lightSt);

		video.GetInputControl()->Subscribe(&player);

		std::thread universeThread(UniverseThread, &universe);

		video.MainLoop();

		universe.Stop();
		universeThread.join();

		video.GetInputControl()->UnSubscribe(&player);

		video.RemoveModel(&field);

		video.RemoveModel(&wallLight);
		video.RemoveModel(&wall1);
		video.RemoveModel(&wall2);
		video.RemoveModel(&wall3);
		video.RemoveModel(&wall4);
		video.RemoveModel(&roof);

		video.RemoveRectangle(&square);

		video.RemoveLight(&lightSt);

		collisionEngine.RemoveObject(&field);
		collisionEngine.RemoveObject(&player);

		collisionEngine.RemoveObject(&wallLight);
		collisionEngine.RemoveObject(&wall1);
		collisionEngine.RemoveObject(&wall2);
		collisionEngine.RemoveObject(&wall3);
		collisionEngine.RemoveObject(&wall4);
		collisionEngine.RemoveObject(&roof);

		universe.RemoveActor(&player);

		universe.RemoveCollisionEngine(&collisionEngine);
	}

	static void UniverseThread(Universe* universe)
	{
		universe->MainLoop();
	}
};

#endif
