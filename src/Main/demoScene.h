#ifndef _DEMO_H
#define _DEMO_H

#include <algorithm>

#include "../VideoEngine/video.h"
#include "../UniverseEngine/universe.h"
#include "../Utils/loader.h"
#include "../Assets/square.h"
#include "../Assets/ExternModel.h"
#include "../Assets/animation.h"
#include "../Assets/ScriptHandler.h"

class Sword : public InputHandler, public Actor
{
public:
	Sword(Video* video)
	{
		_video = video;
		_maxLength = 1.0;

		int texWidth;
		int texHeight;
		auto texData = Loader::LoadImage(
			"../src/Assets/Resources/Models/sword.png",
			texWidth,
			texHeight);

		_swordTexture = video->GetTextures()->AddTexture(
			texWidth,
			texHeight,
			texData);

		texData = Loader::LoadImage(
			"../src/Assets/Resources/Models/blade.png",
			texWidth,
			texHeight);

		_bladeTexture = video->GetTextures()->AddTexture(
			texWidth,
			texHeight,
			texData);

		_state = 0;
		_prevState = 0;
		_length = 0;
		_intensity = 0;

		_sword = new ExternModel(
			"../src/Assets/Resources/Models/sword.obj",
			_swordTexture,
			_swordTexture,
			glm::scale(glm::mat4(1.0f),
				glm::vec3(0.01, 0.01, 0.01)));

		_blade = new ExternModel(
			"../src/Assets/Resources/Models/blade.obj",
			_bladeTexture,
			_bladeTexture,
			glm::scale(glm::mat4(1.0f),
				glm::vec3(0.01, 0.01, 1.0)));
		_blade->SetDrawLight(true);
		_blade->SetDrawLightMultiplier(10);
		_blade->SetDrawEnabled(false);

		video->RegisterModel(_sword);
		video->RegisterModel(_blade);

		_bladeLight1.SetLightColor({0.0, 0.6, 0.0});
		_bladeLight2.SetLightColor({0.0, 0.6, 0.0});
		_bladeLight1.SetLightType(Light::Type::Point);
		_bladeLight2.SetLightType(Light::Type::Point);

		_video->RegisterLight(&_bladeLight1);
		_video->RegisterLight(&_bladeLight2);

		SetInputEnabled(true);
	}

	~Sword()
	{
	}

	void Tick()
	{
		_mutex.lock();

		_intensity += 20;

		if (_intensity >= 360) {
			_intensity -= 360;
		}

		_bladeLight1.SetLightColor(
			glm::vec3(0.0f, 0.3f, 0.0f) *
			(1.0f + sinf(glm::radians(_intensity)) * 0.05f));

		_bladeLight2.SetLightColor(
			glm::vec3(0.0f, 0.3f, 0.0f) *
			(1.0f + sinf(glm::radians(_intensity)) * 0.05f));

		switch (_state) {
		case 1:
			if (_length < _maxLength) {
				_length += 0.02;

				if (_length > _maxLength) {
					_length = _maxLength;
				}


				if (_prevState == 0) {
					_bladeLight1.SetLightActive(true);
					_bladeLight2.SetLightActive(true);
					_blade->SetDrawEnabled(true);
					_prevState = 1;
				}
			}

			break;
		case 0:
			if (_length > 0) {
				_length -= 0.02;

				if (_length < 0) {
					_length = 0;
				}

				if (_length <= 0 && _prevState == 1) {
					_bladeLight1.SetLightActive(false);
					_bladeLight2.SetLightActive(false);
					_blade->SetDrawEnabled(false);
					_prevState = 0;
				}
			}

			break;
		}

		_mutex.unlock();
	}

	void Key(
		int key,
		int scancode,
		int action,
		int mods)
	{
		if (key == GLFW_KEY_F) {
			if (action == GLFW_PRESS) {
				_mutex.lock();

				switch (_state) {
				case 0:
					_state = 1;
					break;
				case 1:
					_state = 0;
					break;
				}

				_mutex.unlock();
			}
		}
	}

	void On()
	{
		_state = 1;
	}

	void Off()
	{
		_state = 0;
	}

	void SetPosition(const glm::mat4 pos)
	{
		_position = pos;

		_bladeLight1.SetLightPosition(ToGlobal({
			0.0,
			0.0,
			_length * 0.33 + 0.35}));

		_bladeLight2.SetLightPosition(ToGlobal({
			0.0,
			0.0,
			_length * 0.66 + 0.35}));

		if (_length > 0 && _length < _maxLength) {
			_bladeLight1.SetLightColor(
				glm::vec3(0.0, 0.3, 0.0) *
				_length / _maxLength);
			_bladeLight2.SetLightColor(
				glm::vec3(0.0, 0.3, 0.0) *
				_length / _maxLength);
		}

		glm::mat4 blade = _position;

		blade = glm::translate(
			blade,
			glm::vec3(0.0, 0.0, 0.35));

		blade = glm::scale(
			blade,
			glm::vec3(
				0.01,
				0.01,
				_length));

		_blade->SetModelMatrix(blade);
		_sword->SetModelMatrix(
			glm::scale(_position, glm::vec3(0.01, 0.01, 0.01)));
	}

private:
	ExternModel* _sword;
	ExternModel* _blade;
	uint32_t _swordTexture;
	uint32_t _bladeTexture;
	Light _bladeLight1;
	Light _bladeLight2;

	int _state;
	int _prevState;
	float _length;
	float _maxLength;

	Video* _video;

	std::mutex _mutex;
	float _intensity;

	glm::mat4 _position;

	glm::vec3 ToGlobal(const glm::vec3& coords)
	{
		return _position * glm::vec4(coords, 1.0f);
	}
};

class Player : public Actor, public Object, public InputHandler
{
public:
	Player(
		Video* video,
		Light* light,
		Sword* sword,
		CollisionEngine* rayEngine)
	{
		_video = video;
		_rayEngine = rayEngine;
		_pos = glm::vec3(0.0, 0.0, 5.0);
		_angleH = 0;
		_angleV = 0;
		_go = 0;
		_strafe = 0;
		_jump = false;
		_vspeed = 0;
		_light = light;
		_lightActive = true;
		_sword = sword;

		// Pyramid
		std::vector<glm::vec3> vertices;
		vertices.push_back(glm::vec3(0.1, -0.1, 0.0));
		vertices.push_back(glm::vec3(0.1, 0.1, 0.0));
		vertices.push_back(glm::vec3(-0.1, 0.0, 0.0));
		vertices.push_back(glm::vec3(0.1, -0.1, 1.0));
		vertices.push_back(glm::vec3(0.1, 0.1, 1.0));
		vertices.push_back(glm::vec3(-0.1, 0.0, 1.0));
		vertices.push_back(glm::vec3(0.1, -0.1, 1.5));
		vertices.push_back(glm::vec3(0.1, 0.1, 1.5));
		vertices.push_back(glm::vec3(-0.1, 0.0, 1.5));
		vertices.push_back(glm::vec3(0.1, -0.1, 2.0));
		vertices.push_back(glm::vec3(0.1, 0.1, 2.0));
		vertices.push_back(glm::vec3(-0.1, 0.0, 2.0));
		vertices.push_back(glm::vec3(0.1, -0.1, 0.5));
		vertices.push_back(glm::vec3(0.1, 0.1, 0.5));
		vertices.push_back(glm::vec3(-0.1, 0.0, 0.5));

		std::vector<uint32_t> indices = {0, 1, 2};

		SetObjectVertices(vertices);
		SetObjectIndices(indices);
		SetObjectCenter({0.0f, 0.0f, 1.5f});
		SetObjectMatrix(glm::mat4(1.0));
		SetObjectDynamic(true);

		SetInputEnabled(true);

		_animation.SetTimePoints(
		{
			{{0.0f, 0.0f, 0.0f}, -30.0f, 0.0f, 0.0f},
			{{0.1f, -0.2f, 0.0f}, -10.0f, 0.0f, -30.0f},
			{{0.0f, 0.0f, 0.0f}, -40.0f, 0.0f, -75.0f},
			{{-0.3f, 0.2f, 0.0f}, -85.0f, 0.0f, 0.0f},
			{{-0.6f, 0.2f, 0.0f}, -120.0f, 0.0f, 75.0f},
			{{0.0f, 0.0f, 0.0f}, -30.0f, 0.0f, 0.0f}
		});

		_animation.SetTimeValues({0, 0.2, 0.8, 1, 1.2, 2.5});
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
					_light->SetLightActive(false);
					_lightActive = false;
				} else {
					_light->SetLightActive(true);
					_lightActive = true;
				}
			}
		} else if (key == GLFW_KEY_E) {
			if (action == GLFW_PRESS) {
				_sword->On();
				_animation.SetStep(0.01);
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

		if (effect.z < 0 && _vspeed > 0) {
			_vspeed = 0;
		}

		if (effect.z > 0 && _jump) {
			_vspeed = 6;
		}

		if (fabs(effect.x) + fabs(effect.y) > 0.0001) {
			if (effect.x * hspeed.x + effect.y * hspeed.y < 0) {
				hspeed = glm::vec2(effect.x, effect.y);
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
				glm::vec3(-hdirStrafe * 0.3f, 0.0f));
		_light->SetLightDirection(glm::vec3(
				hdir * cosf(glm::radians(_angleV)),
				sinf(glm::radians(_angleV))));
		_mutex.unlock();

		_rayEngine->RayCast(
			_pos + glm::vec3(0, 0, 1.85),
			glm::vec3(
				hdir * cosf(glm::radians(_angleV)),
				sinf(glm::radians(_angleV))),
			5,
			nullptr);

		glm::mat4 swp(1.0f);

		swp = glm::translate(
			swp,
			_pos + glm::vec3(hdir * 0.5f + hdirStrafe * 0.3f,
				1.2f));
		swp = glm::rotate(
			swp,
			glm::radians(-_angleH), glm::vec3(0,0,1));

		swp = _animation.Step(swp);

		_sword->SetPosition(swp);
	}

private:
	Video* _video;
	CollisionEngine* _rayEngine;
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

	Sword* _sword;
	Animation _animation;
};

class Field : public Model, public Object
{
public:
	Field(uint32_t textureIndex)
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

		auto model = Loader::LoadModel(
			"../src/Assets/Resources/Models/field.obj");

		for (auto& coord : model.TexCoords) {
			coord *= 100;
		}

		SetModelVertices(model.Vertices);
		SetModelNormals(model.Normals);
		SetModelTexCoords(model.TexCoords);
		SetModelIndices(model.Indices);

		SetTexture({textureIndex});

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

		int texWidth;
		int texHeight;
		auto texData = Loader::LoadImage(
			"../src/Assets/Resources/Models/floor.jpg",
			texWidth,
			texHeight);

		uint32_t woodenTiles = video.GetTextures()->AddTexture(
			texWidth,
			texHeight,
			texData);

		texData = Loader::LoadImage(
			"../src/Assets/Resources/Models/wall.png",
			texWidth,
			texHeight);

		uint32_t wallTexture = video.GetTextures()->AddTexture(
			texWidth,
			texHeight,
			texData);

		texData = Loader::LoadImage(
			"../src/Assets/Resources/Models/wall_specular.png",
			texWidth,
			texHeight);

		uint32_t wallSpecular = video.GetTextures()->AddTexture(
			texWidth,
			texHeight,
			texData);

		texData = Loader::LoadImage(
			"../src/Assets/Resources/Images/transparent.png",
			texWidth,
			texHeight);

		uint32_t squareTexture = video.GetTextures()->AddTexture(
			texWidth,
			texHeight,
			texData);

		Light light;
		light.SetLightType(Light::Type::Spot);
		light.SetLightColor({0.9, 0.9, 0.9});
		light.SetLightAngle(15);
		light.SetLightAngleFade(10);
		light.SetLightActive(true);

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

		Sword sword(&video);
		Player player(&video, &light, &sword, &collisionEngine);
		Field field(woodenTiles);

		glm::mat4 base = glm::translate(
			glm::mat4(1.0),
			glm::vec3(20, 0, 0));

		ExternModel wall1(
			"../src/Assets/Resources/Models/wall.obj",
			wallTexture,
			wallSpecular,
			base);

		base = glm::translate(base, glm::vec3(11, 0, 0));

		ExternModel wall2(
			"../src/Assets/Resources/Models/wall.obj",
			wallTexture,
			wallSpecular,
			base);

		base = glm::rotate(
			base,
			glm::radians(90.0f),
			glm::vec3(0, 0, 1));

		ExternModel wall3(
			"../src/Assets/Resources/Models/wall.obj",
			wallTexture,
			wallSpecular,
			base);

		base = glm::translate(base, glm::vec3(10, 3, 0));

		ExternModel wall4(
			"../src/Assets/Resources/Models/wall.obj",
			wallTexture,
			wallSpecular,
			base);

		ExternModel wallLight(
			"../src/Assets/Resources/Models/wall.obj",
			wallTexture,
			wallSpecular,
			glm::scale(
				glm::translate(
					glm::mat4(1.0f),
				glm::vec3(25, 5, 2.5)),
					glm::vec3(0.1, 0.1, 0.1)));

		ExternModel roof(
			"../src/Assets/Resources/Models/roof.obj",
			wallTexture,
			wallSpecular,
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
		lightSt.SetLightActive(true);

		Square square(squareTexture, 0);

		universe.RegisterActor(&player);
		universe.RegisterActor(&sword);

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
		video.GetInputControl()->Subscribe(&sword);

		auto scene = ScriptHandler::LoadScene(
			"../src/Assets/Scripts/TestScene.script",
			&video);

		for (auto model : scene.Models) {
			video.RegisterModel(model);
			collisionEngine.RegisterObject(model);
		}

		for (auto light : scene.Lights) {
			video.RegisterLight(light);
		}

		std::thread universeThread(UniverseThread, &universe);

		video.MainLoop();

		universe.Stop();
		universeThread.join();

		video.GetInputControl()->UnSubscribe(&player);
		video.GetInputControl()->UnSubscribe(&sword);

		for (auto model : scene.Models) {
			video.RemoveModel(model);
			collisionEngine.RemoveObject(model);
			delete model;
		}

		for (auto light : scene.Lights) {
			video.RemoveLight(light);
			delete light;
		}

		video.RemoveModel(&field);

		video.RemoveModel(&wallLight);
		video.RemoveModel(&wall1);
		video.RemoveModel(&wall2);
		video.RemoveModel(&wall3);
		video.RemoveModel(&wall4);
		video.RemoveModel(&roof);

		video.RemoveRectangle(&square);

		video.RemoveLight(&light);
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
		universe.RemoveActor(&sword);

		universe.RemoveCollisionEngine(&collisionEngine);
	}

	static void UniverseThread(Universe* universe)
	{
		universe->MainLoop();
	}
};

#endif
