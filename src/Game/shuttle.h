#ifndef _SHUTTLE_H
#define _SHUTTLE_H

#include "../VideoEngine/GUI/TextBox.h"
#include "global.h"
#include "common.h"
#include "GravityField.h"

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

	glm::vec3 SetDirection(const glm::vec3& value, const glm::vec3& speed);
	void SetAngle(float value)
	{
		_targetAngle = std::clamp(value, 0.0f, 120.0f);
	}

	float GetFAngle()
	{
		return _angleF;
	}

private:
	glm::vec3 _position;

	float _angle;
	float _targetAngle;

	float _angleF;
	float _angleR;

	Video* _video;
	Model* _exhaust;

	void AdjustFAngle(const glm::vec3& value, const glm::vec3& speed);
	void AdjustRAngle(const glm::vec3& value, const glm::vec3& speed);
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

	glm::vec3 SetSpeed(const glm::vec3& value, const glm::vec3& force);
	void SetAngle(float value)
	{
		_targetAngle = std::clamp(value, 0.0f, 30.0f);
	}

	void IncFlap(float value);
	float GetFlap()
	{
		return _flap;
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

	float _flap;
	float _targetFlap;

	Video* _video;

	bool _inverted;
};

class Shuttle : public InputHandler, public Actor
{
public:
	Shuttle(Common common, GravityField* gf);
	~Shuttle();

	void TickEarly() override;
	void Tick() override;

	void Key(int key, int scancode, int action, int mods) override;
	bool Scroll(double xoffset, double yoffset) override;
	bool MouseMoveRaw(double xoffset, double yoffset) override;
	bool InInputArea(float x, float y) override
	{
		return true;
	}

	void ActivateFlight()
	{
		_flightMode = true;
		_cornerTextBox->Activate();
	}

	const glm::mat4& GetMatrix()
	{
		return _shipMatrix;
	}

private:
	Common _common;

	Library _textures;
	Library _models;

	Model* _base;
	Model* _roof;

	float _mass;

	std::vector<Thruster*> _thrusters;
	std::vector<Wing*> _wings;

	std::vector<Object*> _gear;
	std::vector<glm::vec3> _gearPos;

	GravityField* _gf;

	bool _flightMode;

	glm::vec3 _position;
	glm::mat4 _rotation;
	glm::vec3 _linearSpeed;
	glm::vec3 _angularSpeed;
	glm::mat4 _shipMatrix;

	glm::vec3 _forceMoment;

	glm::vec3 _targetSpeed;
	bool _grounded;

	glm::vec3 _centerOfGravity;

	float _controlF;
	float _controlR;
	float _controlU;

	float _controlYaw;
	float _controlPitch;
	float _controlRoll;

	float _cameraDist;

	TextBox* _cornerTextBox;

	bool _wingsClosed;
	bool _fastMode;
	bool _brakeMode;

	void LoadAssets();
	void UnloadAssets();
	void Flight(int key, int scancode, int action, int mods);

	glm::vec3 GetThrusterForce(
		const glm::vec3& force,
		const glm::vec3& speed,
		float& powerCoeff);
	glm::vec3 GetWingForce(const glm::vec3& speed, const glm::vec3& force);
};

#endif
