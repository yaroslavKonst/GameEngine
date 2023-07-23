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
				_video->GetInputControl()->
					ToggleRawMouseInput();
			}
		} else if (key == GLFW_KEY_W) {
			if (action == GLFW_PRESS) {
				_go += 1;
			} else if (action == GLFW_RELEASE) {
				_go -= 1;
			}
		} else if (key == GLFW_KEY_S) {
			if (action == GLFW_PRESS) {
				_go -= 1;
			} else if (action == GLFW_RELEASE) {
				_go += 1;
			}
		} else if (key == GLFW_KEY_D) {
			if (action == GLFW_PRESS) {
				_strafe += 1;
			} else if (action == GLFW_RELEASE) {
				_strafe -= 1;
			}
		} else if (key == GLFW_KEY_A) {
			if (action == GLFW_PRESS) {
				_strafe -= 1;
			} else if (action == GLFW_RELEASE) {
				_strafe += 1;
			}
		} else if (key == GLFW_KEY_SPACE) {
			if (action == GLFW_PRESS) {
				_jump = true;
			} else if (action == GLFW_RELEASE) {
				_jump = false;
			}
		}
	}

	bool MouseMoveRaw(
		double xoffset,
		double yoffset)
	{
		_angleH += xoffset * 0.1;
		_angleV += yoffset * 0.1;

		if (_angleH < 0) {
			_angleH += 360;
		} else if (_angleH >= 360) {
			_angleH -= 360;
		}

		_angleV = std::clamp(_angleV, -85.0f, 85.0f);

		return true;
	}

	void Tick()
	{
		glm::vec2 hdir(
			sinf(glm::radians(_angleH)),
			cosf(glm::radians(_angleH)));

		glm::vec2 hspeed = hdir * (float)_go;

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
			_vspeed = 8;
		}

		if (effect.x * hspeed.x + effect.y * hspeed.y < 0) {
			hspeed = glm::vec2(0, 0);
		}

		_pos.x += hspeed.x / 20;
		_pos.y += hspeed.y / 20;
		_pos.z += _vspeed / 200;

		SetObjectMatrix(
			glm::rotate(glm::translate(glm::mat4(1.0), _pos),
				glm::radians(_angleH), glm::vec3(0, 0, 1)));

		_video->SetCameraPosition(_pos + glm::vec3(0, 0, 1.85));
		_video->SetCameraDirection(glm::vec3(
				hdir,
				sinf(glm::radians(_angleV))));

		_light->SetLightPosition(_pos + glm::vec3(0, 0, 1.85));
		_light->SetLightDirection(glm::vec3(
				hdir,
				sinf(glm::radians(_angleV))));
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
			{100, -100, 0.251981},
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
			"../src/Assets/Resources/Models/field.png",
			texWidth,
			texHeight);
		auto model = Loader::LoadModel(
			"../src/Assets/Resources/Models/field.obj");

		SetModelVertices(model.Vertices);
		SetModelNormals(model.Normals);
		SetModelTexCoords(model.TexCoords);
		SetModelIndices(model.Indices);

		SetTexWidth(texWidth);
		SetTexHeight(texHeight);
		SetTexData(texture);

		SetDrawEnabled(true);
	}

private:
};

class Column : public Model, public Object
{
public:
private:
};

class Demo
{
public:
	static void Run()
	{
		// 5 ms between universe ticks.
		CollisionEngine collisionEngine;
		Universe universe(5);
		Video video(1400, 1000, "Demo", "Application");

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

		Player player(&video, &light);
		Field field;

		universe.RegisterActor(&player);

		collisionEngine.RegisterObject(&player);
		collisionEngine.RegisterObject(&field);

		video.RegisterModel(&field);

		video.RegisterLight(&light);

		video.GetInputControl()->Subscribe(&player);

		std::thread universeThread(UniverseThread, &universe);

		video.MainLoop();

		universe.Stop();
		universeThread.join();

		video.GetInputControl()->UnSubscribe(&player);

		video.RemoveLight(&light);

		video.RemoveModel(&field);

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
