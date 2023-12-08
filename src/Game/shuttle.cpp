#include "shuttle.h"

#include "../Utils/loader.h"
#include "../PhysicalEngine/PlaneHelper.h"

#include "../Logger/logger.h"

Shuttle::Shuttle(Common common, GravityField* gf)
{
	_common = common;

	_mass = 1000;

	_gf = gf;

	_shipMatrix = glm::mat4(1.0);

	SetInputLayer(1);
	InputArea = {-1, -1, 1, 1};

	_common.video->Subscribe(this);

	_flightMode = false;
	_grounded = false;

	_position = {0, 10, 5};
	_linearSpeed = {0, 0, 0};
	_rotation = glm::mat4(1.0);
	_angularSpeed = {0, 0, 0};

	_targetSpeed = {0, 0, 0};

	_forceMoment = {0, 0, 0};

	_controlF = 0;
	_controlR = 0;
	_controlU = 0;
	_controlYaw = 0;
	_controlPitch = 0;
	_controlRoll = 0;

	_cameraDist = 1.0;

	_wingsClosed = false;
	_fastMode = false;
	_brakeMode = false;

	_centerOfGravity = {0, 20, 0};

	LoadAssets();

	_base = new Model;
	_base->TextureParams.SetAll(_textures["Base"]);
	_base->ModelParams.Model = _models["Base"];
	_base->DrawParams.Enabled = true;
	_base->ModelParams.Matrix = glm::mat4(1.0);
	_base->ModelParams.ExternalMatrix = &_shipMatrix;
	_common.video->RegisterModel(_base);

	_roof = new Model;
	_roof->TextureParams.SetAll(_textures["Roof"]);
	_roof->ModelParams.Model = _models["Roof"];
	_roof->DrawParams.Enabled = true;
	_roof->ModelParams.Matrix = glm::mat4(1.0);
	_roof->ModelParams.ExternalMatrix = &_shipMatrix;
	_common.video->RegisterModel(_roof);

	_thrusters.resize(4, nullptr);

	_thrusters[0] = new Thruster(
		{3.25, 11.07, 1.75},
		_common.video,
		&_shipMatrix,
		&_textures,
		&_models);
	_thrusters[1] = new Thruster(
		{-3.25, 11.07, 1.75},
		_common.video,
		&_shipMatrix,
		&_textures,
		&_models);
	_thrusters[2] = new Thruster(
		{4.65, 35, 1.75},
		_common.video,
		&_shipMatrix,
		&_textures,
		&_models);
	_thrusters[3] = new Thruster(
		{-4.65, 35, 1.75},
		_common.video,
		&_shipMatrix,
		&_textures,
		&_models);

	for (size_t i = 0; i < _thrusters.size(); ++i) {
		_thrusters[i]->TextureParams.SetAll(_textures["Thruster"]);
		_thrusters[i]->ModelParams.Model = _models["Thruster"];
		_thrusters[i]->DrawParams.Enabled = true;
		_thrusters[i]->ModelParams.Matrix = glm::mat4(1.0);
		_thrusters[i]->ModelParams.ExternalMatrix = &_shipMatrix;
		_common.video->RegisterModel(_thrusters[i]);
	}

	_gearPos = {
		{0, 0, -1.6},
		{5, 30, -1.6},
		{-5, 30, -1.6}
	};

	_gear.resize(_gearPos.size());
	auto model = Loader::LoadModel("Models/Ship/MainBlocks/Wall.obj");

	for (size_t idx = 0; idx < _gear.size(); ++idx) {
		Object* gear = new Object;
		gear->SetObjectVertices(model.Vertices);
		gear->SetObjectIndices(model.Indices);
		gear->SetObjectNormals(model.Normals);
		gear->SetObjectCenter();
		gear->SetObjectDynamic(true);
		gear->SetObjectSphereCenter({0, 0, 0.5});
		gear->SetObjectSphereRadius(0.5);
		gear->SetObjectDomain(1);

		gear->SetObjectMatrix(glm::translate(
			glm::mat4(1.0),
			_gearPos[idx]));

		gear->SetObjectExternalMatrix(&_shipMatrix);

		_common.collisionEngine->RegisterObject(gear);

		_gear[idx] = gear;
	}

	_wings.resize(2);
	_wings[0] = new Wing(
		{6.5, 18, 2.5},
		_common.video,
		&_shipMatrix,
		&_textures,
		&_models);
	_wings[0]->TextureParams.SetAll(_textures["Wing"]);
	_wings[0]->ModelParams.Model = _models["Wing"];
	_wings[0]->DrawParams.Enabled = true;
	_wings[0]->ModelParams.Matrix = glm::mat4(1.0);
	_wings[0]->ModelParams.ExternalMatrix = &_shipMatrix;
	_common.video->RegisterModel(_wings[0]);

	_wings[1] = new Wing(
		{-6.5, 18, 2.5},
		_common.video,
		&_shipMatrix,
		&_textures,
		&_models,
		true);
	_wings[1]->TextureParams.SetAll(_textures["Wing"]);
	_wings[1]->ModelParams.Model = _models["WingInverted"];
	_wings[1]->DrawParams.Enabled = true;
	_wings[1]->ModelParams.Matrix = glm::mat4(1.0);
	_wings[1]->ModelParams.ExternalMatrix = &_shipMatrix;
	_wings[1]->ModelParams.InnerMatrix = glm::scale(
		glm::mat4(1.0),
		glm::vec3(-1, 1, 1));
	_common.video->RegisterModel(_wings[1]);

	_cornerTextBox = new TextBox(_common.video, _common.textHandler);
	_cornerTextBox->SetPosition(-0.9, -0.9);
	_cornerTextBox->SetTextSize(0.05);
	_cornerTextBox->SetText("");
	_cornerTextBox->SetTextColor({1, 1, 1, 1});
	_cornerTextBox->SetDepth(0);
}

Shuttle::~Shuttle()
{
	for (Object* gear : _gear) {
		_common.collisionEngine->RemoveObject(gear);
		delete gear;
	}

	_cornerTextBox->Deactivate();
	delete _cornerTextBox;

	_common.video->RemoveModel(_base);
	delete _base;

	_common.video->RemoveModel(_roof);
	delete _roof;

	for (size_t i = 0; i < _thrusters.size(); ++i) {
		_common.video->RemoveModel(_thrusters[i]);
		delete _thrusters[i];
	}

	for (size_t i = 0; i < _wings.size(); ++i) {
		_common.video->RemoveModel(_wings[i]);
		delete _wings[i];
	}

	UnloadAssets();
	_common.video->Unsubscribe(this);
}

void Shuttle::LoadAssets()
{
	auto td = Loader::LoadImage("Models/Shuttle/Base.png");
	_textures["Base"] = _common.video->AddTexture(td);
	auto model = Loader::LoadModel("Models/Shuttle/Base.obj");
	_models["Base"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Roof.png");
	_textures["Roof"] = _common.video->AddTexture(td);
	model = Loader::LoadModel("Models/Shuttle/Roof.obj");
	_models["Roof"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Thruster.png");
	_textures["Thruster"] = _common.video->AddTexture(td);
	model = Loader::LoadModel("Models/Shuttle/Thruster.obj");
	_models["Thruster"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/ThrusterExh.png");
	_textures["ThrusterExh"] = _common.video->AddTexture(td);
	model = Loader::LoadModel("Models/Shuttle/ThrusterExh.obj");
	_models["ThrusterExh"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Wing.png");
	_textures["Wing"] = _common.video->AddTexture(td);
	model = Loader::LoadModel("Models/Shuttle/Wing.obj");
	_models["Wing"] = _common.video->LoadModel(model);

	for (size_t index = 0; index < model.Indices.size(); index += 3) {
		uint32_t tmp = model.Indices[index + 1];
		model.Indices[index + 1] = model.Indices[index + 2];
		model.Indices[index + 2] = tmp;
	}

	_models["WingInverted"] = _common.video->LoadModel(model);
}

void Shuttle::UnloadAssets()
{
	for (auto& texture : _textures) {
		_common.video->RemoveTexture(texture.second);
	}

	for (auto& model : _models) {
		_common.video->UnloadModel(model.second);
	}
}

void Shuttle::TickEarly()
{
	glm::vec3 locDirF(0, -1, 0);
	glm::vec3 locDirR(-1, 0, 0);
	glm::vec3 locDirU(0, 0, 1);

	glm::vec3 dirF = _shipMatrix * glm::vec4(locDirF, 0.0f);
	glm::vec3 dirR = _shipMatrix * glm::vec4(locDirR, 0.0f);
	glm::vec3 dirU = _shipMatrix * glm::vec4(locDirU, 0.0f);

	if (_flightMode) {
		_targetSpeed =
			dirF * _controlF +
			dirR * _controlR +
			dirU * _controlU;

		_targetSpeed *= 10.0f;

		if (_fastMode) {
			_targetSpeed += dirF * _controlF * 1000.0f;
		}
	} else {
		_targetSpeed = {0, 0, 0};

		if (_fastMode) {
			_targetSpeed = dirF * 100.0f;
		}
	}

	_wings[0]->SetAngle(glm::length(_linearSpeed) / 10.0f);
	_wings[1]->SetAngle(glm::length(_linearSpeed) / 10.0f);

	glm::vec3 targetForce = (_targetSpeed - _linearSpeed) * _mass;

	glm::vec3 envForce = -_linearSpeed;

	glm::vec3 mg = (*_gf)(_position) * _mass;

	targetForce -= mg;
	targetForce -= envForce;

	glm::vec3 wingForce = GetWingForce(_linearSpeed, targetForce);
	targetForce -= wingForce;

	if (/*_grounded &&*/ !_flightMode) {
		targetForce = {0, 0, 0};
	}

	float powerCoeff;
	glm::vec3 thrusterForce = GetThrusterForce(
		targetForce,
		_linearSpeed,
		powerCoeff);

	glm::vec3 force =
		mg +
		thrusterForce +
		envForce +
		wingForce;

	_linearSpeed += force / _mass / 100.0f;
	_position += _linearSpeed / 100.0f;

	glm::vec3 targetAngularSpeed =
		(locDirU * _controlYaw +
		locDirR * _controlPitch +
		locDirF * _controlRoll) / 10.0f;

	_controlYaw *= 0.97;
	_controlPitch *= 0.97;
	_controlRoll *= 0.97;

	glm::vec3 forceMoment = _forceMoment;

	_angularSpeed += forceMoment / 100.0f;

	if (_flightMode) {
		_angularSpeed += (targetAngularSpeed * 100.0f - _angularSpeed);
	}

	if (glm::length(_angularSpeed) > 0) {
		_rotation *= glm::rotate(
			glm::mat4(1.0f),
			glm::radians(glm::length(_angularSpeed)) / 100.0f,
			glm::normalize(_angularSpeed));
	}

	_shipMatrix = glm::translate(
		glm::mat4(1.0f),
		_position);

	glm::mat4 rotationMatrix = glm::translate(
		glm::mat4(1.0),
		_centerOfGravity);

	rotationMatrix = _rotation * rotationMatrix;

	rotationMatrix = glm::translate(
		rotationMatrix,
		-_centerOfGravity);

	_shipMatrix = _shipMatrix * rotationMatrix;

	if (_flightMode) {
		glm::vec3 cameraPosition = _position +
			dirU * 10.0f * _cameraDist - dirF * 30.0f * _cameraDist;
		glm::vec3 cameraTarget = _position;

		_common.video->SetCameraPosition(cameraPosition);
		_common.video->SetCameraTarget(cameraTarget);
		_common.video->SetCameraUp(dirU);

		_cornerTextBox->SetText(
			_common.localizer->Localize("FlightData:\nSpeed: ") +
			std::to_string(_linearSpeed.x) + " " +
			std::to_string(_linearSpeed.y) + " " +
			std::to_string(_linearSpeed.z) + "\n" +

			_common.localizer->Localize("Target speed: ") +
			std::to_string(_targetSpeed.x) + " " +
			std::to_string(_targetSpeed.y) + " " +
			std::to_string(_targetSpeed.z) + "\n" +

			_common.localizer->Localize("Flaps: ") +
			std::to_string(_wings[0]->GetFlap()) + "\n" +

			_common.localizer->Localize("F angle: ") +
			std::to_string(_thrusters[0]->GetFAngle()) + "\n" +

			_common.localizer->Localize("Alt: ") +
			std::to_string(_position.z) + "\n" +

			_common.localizer->Localize("Power: ") +
			std::to_string(powerCoeff));
		_cornerTextBox->Activate();
	}
}

void Shuttle::Tick()
{
	glm::vec3 effect(0, 0, 0);
	_forceMoment = {0, 0, 0};

	for (size_t idx = 0; idx < _gear.size(); ++idx) {
		glm::vec3 eff = _gear[idx]->GetObjectEffect();

		if (glm::length(eff) > glm::length(effect)) {
			effect = eff;
		}

		_forceMoment +=
			glm::cross(eff, _centerOfGravity - _gearPos[idx]) *
			1000.0f;
	}

	_position += effect;

	if (glm::dot(effect, _linearSpeed) < 0) {
		_linearSpeed = glm::vec3(0, 0, 0);
		_grounded = true;
	} else {
		_grounded = false;
	}
}

void Shuttle::Key(int key, int scancode, int action, int mods)
{
	if (_flightMode) {
		Flight(key, scancode, action, mods);
	}
}

bool Shuttle::MouseMoveRaw(double xoffset, double yoffset)
{
	if (!_flightMode) {
		return false;
	}

	_controlYaw -= xoffset / 100;
	_controlPitch += yoffset / 100;

	return true;
}

bool Shuttle::Scroll(double xoffset, double yoffset)
{
	if (!_flightMode) {
		return false;
	}

	_cameraDist += yoffset / 10.0;
	return true;
}

void Shuttle::Flight(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) {
			_controlF += 1;
		} else if (action == GLFW_RELEASE) {
			_controlF -= 1;
		}
	} else if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) {
			_controlF -= 1;
		} else if (action == GLFW_RELEASE) {
			_controlF += 1;
		}
	} else if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) {
			_controlR += 1;
		} else if (action == GLFW_RELEASE) {
			_controlR -= 1;
		}
	} else if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
			_controlR -= 1;
		} else if (action == GLFW_RELEASE) {
			_controlR += 1;
		}
	} else if (key == GLFW_KEY_SPACE) {
		if (action == GLFW_PRESS) {
			_controlU += 1;
		} else if (action == GLFW_RELEASE) {
			_controlU -= 1;
		}
	} else if (key == GLFW_KEY_X) {
		if (action == GLFW_PRESS) {
			_controlU -= 1;
		} else if (action == GLFW_RELEASE) {
			_controlU += 1;
		}
	} else if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			_controlRoll -= 0.1;
		}
	} else if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			_controlRoll += 0.1;
		}
	} else if (key == GLFW_KEY_C) {
		if (action == GLFW_PRESS) {
			if (_brakeMode) {
				return;
			}

			if (_wingsClosed) {
				return;
			}

			if (_fastMode) {
				for (size_t i = 0; i < _thrusters.size(); ++i) {
					_thrusters[i]->SetAngle(90);
				}
			} else {
				for (size_t i = 0; i < _thrusters.size(); ++i) {
					_thrusters[i]->SetAngle(0);
				}
			}

			_fastMode = !_fastMode;
		}
	} else if (key == GLFW_KEY_B) {
		if (action == GLFW_PRESS) {
			if (_fastMode) {
				return;
			}

			if (_wingsClosed) {
				return;
			}

			if (_brakeMode) {
				for (size_t i = 0; i < _thrusters.size(); ++i) {
					_thrusters[i]->SetAngle(90);
				}
			} else {
				for (size_t i = 0; i < _thrusters.size(); ++i) {
					_thrusters[i]->SetAngle(120);
				}
			}

			_brakeMode = !_brakeMode;
		}
	} else if (key == GLFW_KEY_U) {
		if (action == GLFW_PRESS) {
			if (_fastMode) {
				return;
			}

			if (_brakeMode) {
				return;
			}

			if (_wingsClosed) {
				for (size_t i = 0; i < _wings.size(); ++i) {
					_wings[i]->Open();
				}
			} else {
				for (size_t i = 0; i < _wings.size(); ++i) {
					_wings[i]->Close();
				}
			}

			_wingsClosed = !_wingsClosed;
		}
	} else if (key == GLFW_KEY_F) {
		if (action == GLFW_PRESS) {
			_targetSpeed = glm::vec3(0.0);
			SetInputEnabled(false);
			_cornerTextBox->Deactivate();
			_flightMode = false;
			return;
		}
	}
}

glm::vec3 Shuttle::GetThrusterForce(
	const glm::vec3& force,
	const glm::vec3& speed,
	float& powerCoeff)
{
	glm::vec3 result(0.0);

	for (auto thruster : _thrusters) {
		result += thruster->SetDirection(force, speed);
	}

	float coeff = glm::length(result) / glm::length(force);

	powerCoeff = 1;

	if (coeff > 1) {
		result /= coeff;
		powerCoeff = 1.0 / coeff;
	}

	if (glm::length(result) < 0.01f) {
		powerCoeff = 0;
	}

	return result;
}

glm::vec3 Shuttle::GetWingForce(const glm::vec3& speed, const glm::vec3& force)
{
	glm::vec3 wingForce =
		_wings[0]->SetSpeed(speed, force) +
		_wings[1]->SetSpeed(speed, force);

	return wingForce;
}

Thruster::Thruster(
	const glm::vec3& position,
	Video* video,
	glm::mat4* extMat,
	Library* textures,
	Library* models)
{
	_position = position;

	_angle = 90;
	_angleF = 0;
	_angleR = 0;

	_targetAngle = 90;

	_video = video;

	_exhaust = new Model;
	_exhaust->TextureParams.SetAll((*textures)["ThrusterExh"]);
	_exhaust->ModelParams.Model = (*models)["ThrusterExh"];
	_exhaust->DrawParams.Enabled = true;
	_exhaust->ModelParams.Matrix = glm::mat4(1.0);
	_exhaust->ModelParams.ExternalMatrix = extMat;
	_video->RegisterModel(_exhaust);
}

Thruster::~Thruster()
{
	_video->RemoveModel(_exhaust);
	delete _exhaust;
}

glm::vec3 Thruster::SetDirection(const glm::vec3& value, const glm::vec3& speed)
{
	if (_angle < _targetAngle) {
		_angle += 0.05;
	} else if (_angle > _targetAngle) {
		_angle -= 0.05;
	}

	_angle = std::clamp(_angle, 0.0f, 120.0f);

	glm::mat4 baseMatrix = glm::translate(glm::mat4(1.0f), _position);
	baseMatrix = glm::rotate(
		baseMatrix,
		glm::radians(_angle),
		glm::vec3(-1, 0, 0));
	ModelParams.Matrix = baseMatrix;

	AdjustFAngle(value, speed);
	AdjustRAngle(value, speed);

	glm::mat4 exhaustMatrix = glm::translate(
		baseMatrix,
		glm::vec3(0, 2, 0));

	exhaustMatrix = glm::rotate(
		exhaustMatrix,
		glm::radians(_angleF),
		glm::vec3(1, 0, 0));

	_exhaust->ModelParams.Matrix = glm::rotate(
		exhaustMatrix,
		glm::radians(_angleR),
		glm::vec3(0, 0, 1));

	glm::vec3 thrustDir(0, -1, 0);
	glm::mat4 rotMatrix = *ModelParams.ExternalMatrix *
		_exhaust->ModelParams.Matrix;
	thrustDir = rotMatrix * glm::vec4(thrustDir, 0.0f);

	float angleCos = glm::dot(
		glm::normalize(thrustDir),
		glm::normalize(value));

	if (angleCos > 0) {
		return glm::normalize(thrustDir) * angleCos * 5000.0f;
	}

	return glm::vec3(0, 0, 0);
}

void Thruster::AdjustFAngle(const glm::vec3& value, const glm::vec3& speed)
{
	if (_angle < 30 && glm::length(speed) > 30.0f) {
		if (_angleF > 0.1) {
			_angleF -= 0.1;
		} else if (_angleF < -0.1) {
			_angleF += 0.1;
		} else {
			_angleF = 0;
		}

		return;
	}

	if (_angle < 30 && _angleF > 0) {
		_angleF -= 0.1;
		return;
	}

	glm::vec3 planePoint1(0, 0, 0);
	glm::vec3 planePoint2(0, 1, 0);
	glm::vec3 planePoint3(0, 1, 1);
	glm::vec3 normal(-1, 0, 0);

	glm::vec3 thrustDir(0, -1, 0);

	glm::mat4 matrix = *ModelParams.ExternalMatrix * ModelParams.Matrix;
	glm::mat4 rotMatrix = *ModelParams.ExternalMatrix *
		_exhaust->ModelParams.Matrix;

	planePoint1 = matrix * glm::vec4(planePoint1, 1.0f);
	planePoint2 = matrix * glm::vec4(planePoint2, 1.0f);
	planePoint3 = matrix * glm::vec4(planePoint3, 1.0f);
	normal = matrix * glm::vec4(normal, 0.0f);
	thrustDir = rotMatrix * glm::vec4(thrustDir, 0.0f);

	PlaneHelper::Plane enginePlane =
		PlaneHelper::PlaneByThreePoints(
			planePoint1,
			planePoint2,
			planePoint3);

	enginePlane[3] = 0;

	glm::vec3 valueDir =
		PlaneHelper::ProjectPointToPlane(value, enginePlane);
	thrustDir =
		PlaneHelper::ProjectPointToPlane(thrustDir, enginePlane);

	valueDir = glm::normalize(valueDir);
	thrustDir = glm::normalize(thrustDir);

	glm::vec3 spinDir = glm::cross(valueDir, thrustDir);
	float dot = glm::dot(valueDir, thrustDir);

	float crossDot = glm::dot(spinDir, glm::normalize(normal));

	if (crossDot > 0) {
		if (dot < 0 || fabs(crossDot) > 0.2) {
			_angleF += 1;
		} else {
			_angleF += 0.1;
		}
	} else if (crossDot < 0) {
		if (dot < 0 || fabs(crossDot) > 0.2) {
			_angleF -= 1;
		} else {
			_angleF -= 0.1;
		}
	}

	_angleF = std::clamp(_angleF, -30.0f, 30.0f);
}

void Thruster::AdjustRAngle(const glm::vec3& value, const glm::vec3& speed)
{
	if (_angle < 30) {
		if (_angleR > 0.1) {
			_angleR -= 0.1;
		} else if (_angleR < -0.1) {
			_angleR += 0.1;
		} else {
			_angleR = 0;
		}

		return;
	}

	glm::vec3 planePoint1(0, 0, 0);
	glm::vec3 planePoint2(1, 0, 0);
	glm::vec3 planePoint3(1, 1, 0);
	glm::vec3 normal(0, 0, -1);

	glm::vec3 thrustDir(0, -1, 0);

	glm::mat4 matrix = *ModelParams.ExternalMatrix * ModelParams.Matrix;
	glm::mat4 rotMatrix = *ModelParams.ExternalMatrix *
		_exhaust->ModelParams.Matrix;

	planePoint1 = matrix * glm::vec4(planePoint1, 1.0f);
	planePoint2 = matrix * glm::vec4(planePoint2, 1.0f);
	planePoint3 = matrix * glm::vec4(planePoint3, 1.0f);
	normal = matrix * glm::vec4(normal, 0.0f);
	thrustDir = rotMatrix * glm::vec4(thrustDir, 0.0f);

	PlaneHelper::Plane enginePlane =
		PlaneHelper::PlaneByThreePoints(
			planePoint1,
			planePoint2,
			planePoint3);

	enginePlane[3] = 0;

	glm::vec3 valueDir =
		PlaneHelper::ProjectPointToPlane(value, enginePlane);
	thrustDir =
		PlaneHelper::ProjectPointToPlane(thrustDir, enginePlane);

	valueDir = glm::normalize(valueDir);
	thrustDir = glm::normalize(thrustDir);

	glm::vec3 spinDir = glm::cross(valueDir, thrustDir);
	float dot = glm::dot(valueDir, thrustDir);

	float crossDot = glm::dot(spinDir, glm::normalize(normal));

	if (crossDot > 0) {
		if (dot < 0 || fabs(crossDot) > 0.2) {
			_angleR += 1;
		} else {
			_angleR += 0.1;
		}
	} else if (crossDot < 0) {
		if (dot < 0 || fabs(crossDot) > 0.2) {
			_angleR -= 1;
		} else {
			_angleR -= 0.1;
		}
	}

	_angleR = std::clamp(_angleR, -30.0f, 30.0f);
}

Wing::Wing(
	const glm::vec3& position,
	Video* video,
	glm::mat4* extMat,
	Library* textures,
	Library* models,
	bool inverted)
{
	_position = position;
	_video = video;
	_inverted = inverted;

	_angle = 0;
	_targetAngle = 0;

	_angleUp = 0;
	_targetAngleUp = 0;

	_flap = 0;
	_targetFlap = 0;
}

Wing::~Wing()
{
}

void Wing::IncFlap(float value)
{
	_targetFlap += value;
	_targetFlap = std::clamp(_targetFlap, 0.0f, 1.0f);
}

static glm::vec3 ProjVector(
	const glm::vec3& projectedVector,
	const glm::vec3& onProjVector)
{
	return onProjVector *
		glm::dot(projectedVector, onProjVector) /
		glm::dot(onProjVector, onProjVector);
}

glm::vec3 Wing::SetSpeed(const glm::vec3& value, const glm::vec3& force)
{
	if (_angle < _targetAngle && _angleUp == 0) {
		_angle += 0.05;
	}
	
	if (_angle > _targetAngle) {
		_angle -= 0.05;
	}

	if (_flap < _targetFlap) {
		_flap += 0.001;
	} else if (_flap > _targetFlap) {
		_flap -= 0.001;
	}

	_angle = std::clamp(_angle, 0.0f, 30.0f);

	if (_angleUp < _targetAngleUp && _angle < 1) {
		_angleUp += 0.05;
	}
	
	if (_angleUp > _targetAngleUp) {
		_angleUp -= 0.05;
	}

	_angleUp = std::clamp(_angleUp, 0.0f, 90.0f);

	glm::mat4 matrix = glm::translate(
		glm::mat4(1.0),
		_position);

	matrix = glm::rotate(
		matrix,
		glm::radians(_angle),
		glm::vec3(0, 0, _inverted ? -1 : 1));
	matrix = glm::rotate(
		matrix,
		glm::radians(_angleUp),
		glm::vec3(0, _inverted ? 1 : -1, 0));

	ModelParams.Matrix = matrix;

	if (glm::length(value) < 0.00001) {
		return glm::vec3(0);
	}

	glm::vec3 forward =
		*ModelParams.ExternalMatrix * glm::vec4(0, -1, 0, 0);
	glm::vec3 right = *ModelParams.ExternalMatrix * glm::vec4(-1, 0, 0, 0);
	glm::vec3 up = *ModelParams.ExternalMatrix * glm::vec4(0, 0, 1, 0);

	forward = glm::normalize(forward);
	right = glm::normalize(right);
	up = glm::normalize(up);

	glm::vec3 speedProj = value - ProjVector(value, forward);
	glm::vec3 forceProj = force - ProjVector(force, forward);

	glm::vec3 effect = -glm::normalize(speedProj);
	effect += up * _flap * 0.3f;

	float speedFactor = glm::length(value) * 200.0f;

	effect *= speedFactor * (1.0f - _angleUp / 90.0f);

	float forceLenDiff = glm::length(effect) - glm::length(forceProj);
	bool forceUp = glm::dot(up, forceProj) > 0;

	if (forceLenDiff > 0 || !forceUp) {
		IncFlap(-0.01);
	} else if (forceLenDiff < 0) {
		IncFlap(0.01);
	}

	return effect;
}
