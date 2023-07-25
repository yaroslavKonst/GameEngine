#ifndef _DEMO_H
#define _DEMO_H

#include <algorithm>

#include "../VideoEngine/video.h"
#include "../UniverseEngine/universe.h"
#include "../Utils/loader.h"

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
		Object::CollisionPrimitive primitive;
		primitive.Vertices[0] = glm::vec3(0.1, -0.1, 0.0);
		primitive.Vertices[1] = glm::vec3(0.1, 0.1, 0.0);
		primitive.Vertices[2] = glm::vec3(-0.1, 0.0, 0.0);
		primitive.Vertices[3] = glm::vec3(0.0, 0.0, 2.0);

		SetCollisionPrimitives({primitive});
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
				_vspeed = 0;
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
		SetObjectSpeed(glm::vec3(0.0, 0.0, _vspeed / 200));

		_video->SetCameraPosition(_pos + glm::vec3(0, 0, 1.85));
		_video->SetCameraDirection(glm::vec3(
				hdir,
				sinf(glm::radians(_angleV))));

		_light->SetLightPosition(_pos + glm::vec3(0, 0, 1.85));
		_light->SetLightDirection(glm::vec3(
				hdir,
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
			{-200, -200, 0.251981},
			{-200, 200, 0.251981},
			{200, 200, 0.251981},
			{200, -200, 0.251981},
			{0, 0, -300},
		};
		// 2 pyramids.
		std::vector<Object::CollisionPrimitive> primitives(2);
		primitives[0].Vertices[0] = objectVertices[0];
		primitives[0].Vertices[1] = objectVertices[1];
		primitives[0].Vertices[2] = objectVertices[2];
		primitives[0].Vertices[3] = objectVertices[4];
		primitives[1].Vertices[0] = objectVertices[0];
		primitives[1].Vertices[1] = objectVertices[2];
		primitives[1].Vertices[2] = objectVertices[3];
		primitives[1].Vertices[3] = objectVertices[4];

		SetCollisionPrimitives(primitives);
		SetObjectMatrix(glm::mat4(1.0));
		SetModelMatrix(glm::scale(
			glm::mat4(1.0),
			glm::vec3(10, 10, 1)));
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

class Brick : public Model
{
public:
	Brick()
	{
		SetModelMatrix(glm::translate(
			glm::mat4(1.0),
			glm::vec3(0, 50, 5)));
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
		Video video(1400, 1000, "Demo", "Application");

		video.SetSceneMutex(&sceneMutex);
		universe.SetSceneMutex(&sceneMutex);

		universe.RegisterCollisionEngine(&collisionEngine);

		Light light;
		light.SetLightType(Light::Type::Spot);
		light.SetLightColor({0.9, 0.9, 0.9});
		light.SetLightAngle(15);
		light.SetLightAngleFade(10);

		Light lightSt1;
		lightSt1.SetLightType(Light::Type::Point);
		lightSt1.SetLightColor({0.9, 0.9, 0.9});
		lightSt1.SetLightPosition({0.0, 0.0, 3.0});

		Light lightSt2;
		lightSt2.SetLightType(Light::Type::Spot);
		lightSt2.SetLightColor({0.9, 0.0, 0.0});
		lightSt2.SetLightPosition({20.0, 20.0, 3.0});
		lightSt2.SetLightDirection({0.0, 0.0, -1.0});
		lightSt2.SetLightAngle(70);
		lightSt2.SetLightAngleFade(10);

		Light lightSt3;
		lightSt3.SetLightType(Light::Type::Point);
		lightSt3.SetLightColor({2000.0, 2000.0, 2000.0});
		lightSt3.SetLightPosition({0.0, 0.0, 200.0});

		video.SetFOV(80);
		video.SetCameraUp({0, 0, 1});
		int skbTWidth;
		int skbTHeight;
		auto skbTData = Loader::LoadImage(
			"../src/Assets/Resources/Skybox/skybox.png",
			skbTWidth,
			skbTHeight);
		video.CreateSkybox(skbTWidth, skbTHeight, skbTData);

		Player player(&video, &light);
		Field field;
		Brick brick;

		universe.RegisterActor(&player);

		collisionEngine.RegisterObject(&player);
		collisionEngine.RegisterObject(&field);

		video.RegisterModel(&field);
		video.RegisterModel(&brick);

		video.RegisterLight(&light);
		video.RegisterLight(&lightSt1);
		video.RegisterLight(&lightSt2);
		video.RegisterLight(&lightSt3);

		video.GetInputControl()->Subscribe(&player);

		std::thread universeThread(UniverseThread, &universe);

		video.MainLoop();

		universe.Stop();
		universeThread.join();

		video.GetInputControl()->UnSubscribe(&player);

		video.RemoveLight(&lightSt1);
		video.RemoveLight(&lightSt2);
		video.RemoveLight(&lightSt3);

		video.RemoveModel(&field);
		video.RemoveModel(&brick);

		collisionEngine.RemoveObject(&field);
		collisionEngine.RemoveObject(&player);

		universe.RemoveActor(&player);

		universe.RemoveCollisionEngine(&collisionEngine);
	}

	static void UniverseThread(Universe* universe)
	{
		universe->MainLoop();
	}
};

#endif
