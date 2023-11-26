#ifndef _SHUTTLE_H
#define _SHUTTLE_H

#include "../VideoEngine/video.h"
#include "../PhysicalEngine/CollisionEngine.h"
#include "../UniverseEngine/actor.h"
#include "common.h"

class Thruster : public Model
{
public:
	Thruster(
		const glm::vec3& position,
		Video* video,
		glm::mat4* extMat,
		Library* textures,
		Library* models);
	~Thruster();

	glm::vec3 SetDirection(const glm::vec3& value);
	void SetAngle(float value)
	{
		_targetAngle = std::clamp(value, 0.0f, 90.0f);
	}

private:
	glm::vec3 _position;

	float _angle;
	float _targetAngle;

	float _angleF;
	float _angleR;

	Video* _video;
	Model* _exhaust;

	void AdjustFAngle(const glm::vec3& value);
	void AdjustRAngle(const glm::vec3& value);
};

class Wing : public Model
{
public:
	Wing(
		const glm::vec3& position,
		Video* video,
		glm::mat4* extMat,
		Library* textures,
		Library* models,
		bool inverted = false);
	~Wing();

	glm::vec3 SetSpeed(const glm::vec3& value);
	void SetAngle(float value)
	{
		_targetAngle = std::clamp(value, 0.0f, 30.0f);
	}

	void Open()
	{
		_targetAngleUp = 0;
	}

	void Close()
	{
		_targetAngleUp = 90;
	}

private:
	glm::vec3 _position;

	float _angle;
	float _targetAngle;

	float _angleUp;
	float _targetAngleUp;

	Video* _video;

	bool _inverted;
};

class Shuttle : public InputHandler, public Actor
{
public:
	Shuttle(Video* video, CollisionEngine* collisionEngine);
	~Shuttle();

	void TickEarly() override;
	void Tick() override;

	void Key(int key, int scancode, int action, int mods) override;
	bool Scroll(double xoffset, double yoffset) override;
	bool MouseMoveRaw(double xoffset, double yoffset) override;

	void ActivateFlight()
	{
		_flightMode = true;
	}

	const glm::mat4& GetMatrix()
	{
		return _shipMatrix;
	}

private:
	Library _textures;
	Library _models;

	Model* _base;
	Model* _roof;

	std::vector<Thruster*> _thrusters;
	std::vector<Wing*> _wings;

	Object* _gear;

	Video* _video;
	CollisionEngine* _collisionEngine;

	bool _flightMode;

	glm::vec3 _position;
	glm::vec3 _rotation;
	glm::vec3 _linearSpeed;
	glm::vec3 _angularSpeed;
	glm::mat4 _shipMatrix;

	glm::vec3 _targetSpeed;
	bool _grounded;

	glm::vec3 _centerOfGravity;

	float _controlF;
	float _controlR;
	float _controlU;

	float _controlYaw;
	float _controlPitch;

	bool _wingsClosed;
	bool _fastMode;

	void LoadAssets();
	void UnloadAssets();
	void Flight(int key, int scancode, int action, int mods);

	glm::vec3 GetThrusterForce(const glm::vec3& force);
	glm::vec3 GetWingForce(const glm::vec3& speed);
};

#endif
