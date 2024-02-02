#ifndef _SHUTTLE_H
#define _SHUTTLE_H

#include "../Engine/Video/GUI/TextBox.h"
#include "global.h"
#include "common.h"
#include "GravityField.h"

class Thruster : public Model
{
public:
	Thruster(
		const Math::Vec<3>& position,
		Video* video,
		Math::Mat<4>* extMat,
		Library* textures,
		Library* models);
	~Thruster();

	Math::Vec<3> SetDirection(
		const Math::Vec<3>& value,
		const Math::Vec<3>& speed);
	void SetAngle(double value)
	{
		_targetAngle = std::clamp(value, 0.0, 120.0);
	}

	double GetFAngle()
	{
		return _angleF;
	}

private:
	Math::Vec<3> _position;

	double _angle;
	double _targetAngle;

	double _angleF;
	double _angleR;

	Video* _video;
	Model* _exhaust;

	void AdjustFAngle(const Math::Vec<3>& value, const Math::Vec<3>& speed);
	void AdjustRAngle(const Math::Vec<3>& value, const Math::Vec<3>& speed);
};

class Wing : public Model
{
public:
	Wing(
		const Math::Vec<3>& position,
		Video* video,
		Math::Mat<4>* extMat,
		Library* textures,
		Library* models,
		bool inverted = false);
	~Wing();

	Math::Vec<3> SetSpeed(
		const Math::Vec<3>& value,
		const Math::Vec<3>& force);
	void SetAngle(double value)
	{
		_targetAngle = std::clamp(value, 0.0, 30.0);
	}

	void IncFlap(double value);
	double GetFlap()
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
	Math::Vec<3> _position;

	double _angle;
	double _targetAngle;

	double _angleUp;
	double _targetAngleUp;

	double _flap;
	double _targetFlap;

	Video* _video;

	bool _inverted;
};

class Shuttle : public InputHandler, public Actor, public SoftObject
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

	const Math::Mat<4>& GetMatrix()
	{
		return _shipMatrix;
	}

private:
	Common _common;

	Library _textures;
	Library _models;

	Model* _base;
	Model* _roof;

	std::vector<Thruster*> _thrusters;
	std::vector<Wing*> _wings;

	std::vector<Math::Vec<3>> _gearPos;
	std::vector<Math::Vec<3>> _thrusterPos;
	std::vector<Sprite*> _sprites;

	GravityField* _gf;

	bool _flightMode;

	Math::Mat<4> _shipMatrix;

	bool _grounded;

	double _controlF;
	double _controlR;
	double _controlU;

	double _controlYaw;
	double _controlPitch;
	double _controlRoll;

	double _cameraDist;

	TextBox* _cornerTextBox;

	bool _wingsClosed;
	bool _fastMode;
	bool _brakeMode;

	void LoadAssets();
	void UnloadAssets();
	void Flight(int key, int scancode, int action, int mods);

	void SetThrusterForce(
		const Math::Vec<3>& force,
		const Math::Vec<3>& speed,
		const Math::Vec<3>& dirF,
		const Math::Vec<3>& dirR,
		const Math::Vec<3>& dirU);
	Math::Vec<3> GetWingForce(
		const Math::Vec<3>& speed,
		const Math::Vec<3>& force);

	void BuildPhysicalFrame();
	void RemovePhysicalFrame();
};

#endif
