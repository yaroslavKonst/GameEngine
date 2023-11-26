#include "shuttle.h"

#include "../Utils/loader.h"
#include "../PhysicalEngine/PlaneHelper.h"

#include "../Logger/logger.h"

Shuttle::Shuttle(Video* video, CollisionEngine* collisionEngine)
{
	_video = video;
	_collisionEngine = collisionEngine;

	_shipMatrix = glm::mat4(1.0);

	SetInputLayer(1);
	InputArea = {-1, -1, 1, 1};

	_video->GetInputControl()->Subscribe(this);

	_flightMode = false;
	_grounded = false;

	_position = {0, 0, 5};
	_linearSpeed = {0, 0, 0};
	_rotation = {0, 0, 0};
	_angularSpeed = {0, 0, 0};

	_targetSpeed = {0, 0, 0};

	_controlF = 0;
	_controlR = 0;
	_controlU = 0;
	_controlYaw = 0;
	_controlPitch = 0;

	_wingsClosed = false;
	_fastMode = false;

	_centerOfGravity = {0, 20, 0};

	LoadAssets();

	_base = new Model;
	_base->SetTexture({_textures["Base"]});
	_base->SetModels({_models["Base"]});
	_base->SetDrawEnabled(true);
	_base->SetModelMatrix(glm::mat4(1.0));
	_base->SetModelExternalMatrix(&_shipMatrix);
	_base->SetModelInnerMatrix(glm::mat4(1.0));
	_video->RegisterModel(_base);

	_roof = new Model;
	_roof->SetTexture({_textures["Roof"]});
	_roof->SetModels({_models["Roof"]});
	_roof->SetDrawEnabled(true);
	_roof->SetModelMatrix(glm::mat4(1.0));
	_roof->SetModelExternalMatrix(&_shipMatrix);
	_roof->SetModelInnerMatrix(glm::mat4(1.0));
	_video->RegisterModel(_roof);

	_thrusters.resize(4, nullptr);

	_thrusters[0] = new Thruster(
		{3.25, 11.07, 1.75},
		_video,
		&_shipMatrix,
		&_textures,
		&_models);
	_thrusters[1] = new Thruster(
		{-3.25, 11.07, 1.75},
		_video,
		&_shipMatrix,
		&_textures,
		&_models);
	_thrusters[2] = new Thruster(
		{4.65, 35, 1.75},
		_video,
		&_shipMatrix,
		&_textures,
		&_models);
	_thrusters[3] = new Thruster(
		{-4.65, 35, 1.75},
		_video,
		&_shipMatrix,
		&_textures,
		&_models);

	for (size_t i = 0; i < _thrusters.size(); ++i) {
		_thrusters[i]->SetTexture({_textures["Thruster"]});
		_thrusters[i]->SetModels({_models["Thruster"]});
		_thrusters[i]->SetDrawEnabled(true);
		_thrusters[i]->SetModelMatrix(glm::mat4(1.0));
		_thrusters[i]->SetModelExternalMatrix(&_shipMatrix);
		_thrusters[i]->SetModelInnerMatrix(glm::mat4(1.0));
		_video->RegisterModel(_thrusters[i]);
	}

	_gear = new Object;
	auto model = Loader::LoadModel("Models/Ship/MainBlocks/Wall.obj");
	_gear->SetObjectVertices(model.Vertices);
	_gear->SetObjectIndices(model.Indices);
	_gear->SetObjectNormals(model.Normals);
	_gear->SetObjectCenter();
	_gear->SetObjectDynamic(true);
	_gear->SetObjectSphereCenter({0, 0, 0.5});
	_gear->SetObjectSphereRadius(0.5);
	_gear->SetObjectDomain(1);
	_gear->SetObjectMatrix(glm::translate(
		glm::mat4(1.0),
		glm::vec3(0, 0, -1)));
	_gear->SetObjectExternalMatrix(&_shipMatrix);

	_collisionEngine->RegisterObject(_gear);

	_wings.resize(2);
	_wings[0] = new Wing(
		{6.5, 18, 2.5},
		_video,
		&_shipMatrix,
		&_textures,
		&_models);
	_wings[0]->SetTexture({_textures["Wing"]});
	_wings[0]->SetModels({_models["Wing"]});
	_wings[0]->SetDrawEnabled(true);
	_wings[0]->SetModelMatrix(glm::mat4(1.0));
	_wings[0]->SetModelExternalMatrix(&_shipMatrix);
	_wings[0]->SetModelInnerMatrix(glm::mat4(1.0));
	_video->RegisterModel(_wings[0]);

	_wings[1] = new Wing(
		{-6.5, 18, 2.5},
		_video,
		&_shipMatrix,
		&_textures,
		&_models,
		true);
	_wings[1]->SetTexture({_textures["Wing"]});
	_wings[1]->SetModels({_models["WingInverted"]});
	_wings[1]->SetDrawEnabled(true);
	_wings[1]->SetModelMatrix(glm::mat4(1.0));
	_wings[1]->SetModelExternalMatrix(&_shipMatrix);
	_wings[1]->SetModelInnerMatrix(glm::scale(
		glm::mat4(1.0),
		glm::vec3(-1, 1, 1)));
	_video->RegisterModel(_wings[1]);
}

Shuttle::~Shuttle()
{
	_collisionEngine->RemoveObject(_gear);

	_video->RemoveModel(_base);
	delete _base;

	_video->RemoveModel(_roof);
	delete _roof;

	for (size_t i = 0; i < _thrusters.size(); ++i) {
		_video->RemoveModel(_thrusters[i]);
		delete _thrusters[i];
	}

	for (size_t i = 0; i < _wings.size(); ++i) {
		_video->RemoveModel(_wings[i]);
		delete _wings[i];
	}

	UnloadAssets();
	_video->GetInputControl()->UnSubscribe(this);
}

void Shuttle::LoadAssets()
{
	int tw;
	int th;
	auto td = Loader::LoadImage("Models/Shuttle/Base.png", tw, th);
	_textures["Base"] = _video->GetTextures()->AddTexture(tw, th, td);
	auto model = Loader::LoadModel("Models/Shuttle/Base.obj");
	_models["Base"] = _video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Roof.png", tw, th);
	_textures["Roof"] = _video->GetTextures()->AddTexture(tw, th, td);
	model = Loader::LoadModel("Models/Shuttle/Roof.obj");
	_models["Roof"] = _video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Thruster.png", tw, th);
	_textures["Thruster"] = _video->GetTextures()->AddTexture(tw, th, td);
	model = Loader::LoadModel("Models/Shuttle/Thruster.obj");
	_models["Thruster"] = _video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/ThrusterExh.png", tw, th);
	_textures["ThrusterExh"] =
		_video->GetTextures()->AddTexture(tw, th, td);
	model = Loader::LoadModel("Models/Shuttle/ThrusterExh.obj");
	_models["ThrusterExh"] = _video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Wing.png", tw, th);
	_textures["Wing"] =
		_video->GetTextures()->AddTexture(tw, th, td);
	model = Loader::LoadModel("Models/Shuttle/Wing.obj");
	_models["Wing"] = _video->LoadModel(model);

	for (size_t index = 0; index < model.Indices.size(); index += 3) {
		uint32_t tmp = model.Indices[index + 1];
		model.Indices[index + 1] = model.Indices[index + 2];
		model.Indices[index + 2] = tmp;
	}

	_models["WingInverted"] = _video->LoadModel(model);
}

void Shuttle::UnloadAssets()
{
	for (auto& texture : _textures) {
		_video->GetTextures()->RemoveTexture(texture.second);
	}

	for (auto& model : _models) {
		_video->UnloadModel(model.second);
	}
}

void Shuttle::TickEarly()
{
	glm::vec3 dirF = _shipMatrix *
		glm::vec4(0, -1, 0, 0);
	glm::vec3 dirR = _shipMatrix *
		glm::vec4(-1, 0, 0, 0);
	glm::vec3 dirU = _shipMatrix *
		glm::vec4(0, 0, 1, 0);

	if (_flightMode) {
		_targetSpeed =
			dirF * _controlF +
			dirR * _controlR +
			dirU * _controlU;
	} else {
		_targetSpeed = {0, 0, 0};

		if (glm::length(_linearSpeed) > 0.15) {
			_targetSpeed = dirF * 100.0f;

			if (_position.z < 5) {
				_targetSpeed.z += 100;
			}
		}
	}

	_wings[0]->SetAngle(glm::length(_linearSpeed) * 400);
	_wings[1]->SetAngle(glm::length(_linearSpeed) * 400);

	glm::vec3 mg(0, 0, -1);

	glm::vec3 thrusterForce = {0, 0, 0};
	glm::vec3 targetForce = _targetSpeed / 10.0f - _linearSpeed;

	targetForce -= mg;

	glm::vec3 wingForce = GetWingForce(_linearSpeed) * 100.0f;
	targetForce -= wingForce;

	targetForce += _linearSpeed;

	thrusterForce = GetThrusterForce(targetForce);

	glm::vec3 force = mg + thrusterForce - _linearSpeed + wingForce;

	glm::vec3 targetAngularSpeed =
		(dirU * _controlYaw + dirR * _controlPitch) / 10.0f;
	glm::vec3 forceMoment = targetAngularSpeed - _angularSpeed;

	_linearSpeed += force / 100.0f;
	_angularSpeed += forceMoment / 100.0f;
	_position += _linearSpeed;
	_rotation += _angularSpeed;

	_shipMatrix = glm::translate(
		glm::mat4(1.0f),
		_position);

	glm::mat4 rotationMatrix = glm::translate(
		glm::mat4(1.0),
		_centerOfGravity);

	if (glm::length(_rotation) > 0) {
		rotationMatrix = glm::rotate(
			rotationMatrix,
			glm::radians(glm::length(_rotation)),
			glm::normalize(_rotation));
	}

	rotationMatrix = glm::translate(
		rotationMatrix,
		-_centerOfGravity);

	_shipMatrix = _shipMatrix * rotationMatrix;
}

void Shuttle::Tick()
{
	glm::vec3 effect = _gear->GetObjectEffect();

	_position += effect;

	if (glm::dot(effect, _linearSpeed) < 0) {
		_linearSpeed = glm::vec3(0, 0, 0);
		_grounded = true;
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

	_controlYaw += xoffset / 100;
	_controlPitch += yoffset / 100;

	Logger::Verbose() << "Yaw: " << _controlYaw << ", pitch: " <<
		_controlPitch;

	return true;

	/*glm::vec3 dirU = _shipMatrix *
		glm::vec4(0, 0, 1, 0);

	glm::vec3 dirR = _shipMatrix *
		glm::vec4(1, 0, 0, 0);

	glm::vec3 rotation =
		dirU * (float)xoffset / 10.0f;// + dirR * (float)yoffset / 10.0f;

	glm::mat4 R(1.0);

	if (glm::length(_targetAngle) > 0) {
		R = glm::rotate(
			glm::mat4(1.0),
			glm::radians(glm::length(_targetAngle)),
			glm::normalize(_targetAngle));
	}

	if (glm::length(rotation) > 0) {
		R = glm::rotate(
			R,
			glm::radians(glm::length(rotation)),
			glm::normalize(rotation));
	}

	rotation = {
		R[2][1] - R[1][2],
		R[0][2] - R[2][0],
		R[1][0] - R[0][1]
	};

	if (glm::length(rotation) == 0) {
		_targetAngle = {0, 0, 0};
		return true;
	}

	rotation = glm::normalize(rotation);

	glm::mat4 Kn(0.0f);

	Kn[1][0] = rotation.z;
	Kn[2][0] = -rotation.y;
	Kn[2][1] = rotation.x;

	Kn[0][1] = -Kn[1][0];
	Kn[0][2] = -Kn[2][0];
	Kn[1][2] = -Kn[2][1];

	Kn = Kn * R;

	float traceR = R[0][0] + R[1][1] + R[2][2];
	float traceKn = Kn[0][0] + Kn[1][1] + Kn[2][2];

	float cosTheta = (traceR - 1.0) / 2.0;
	float sinTheta = -traceKn / 2.0;

	float angle = asin(sinTheta);

	if (cosTheta < 0) {
		if (angle < 0) {
			angle -= M_PI / 2.0;
		} else {
			angle += M_PI / 2.0;
		}
	}

	rotation = rotation * angle * 180.0f / (float)M_PI;

	Logger::Verbose() << "Rotation: " << rotation.x << " " <<
		rotation.y << " " << rotation.z;
	Logger::Verbose() << "Angle: " << angle;

	_targetAngle = rotation;

	return true;*/
}

bool Shuttle::Scroll(double xoffset, double yoffset)
{
	return false;
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
	} else if (key == GLFW_KEY_C) {
		if (action == GLFW_PRESS) {
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
	} else if (key == GLFW_KEY_U) {
		if (action == GLFW_PRESS) {
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
	} else if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS) {
			_targetSpeed = glm::vec3(0.0);
			SetInputEnabled(false);
			_flightMode = false;
			return;
		}
	}
}

glm::vec3 Shuttle::GetThrusterForce(const glm::vec3& force)
{
	glm::vec3 result(0.0);

	for (auto thruster : _thrusters) {
		result += thruster->SetDirection(force);
	}

	float coeff = glm::length(result) / glm::length(force);

	if (coeff > 1) {
		result /= coeff;
	}

	return result;
}

glm::vec3 Shuttle::GetWingForce(const glm::vec3& speed)
{
	glm::vec3 wingForce =
		_wings[0]->SetSpeed(speed) + _wings[1]->SetSpeed(speed);

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
	_exhaust->SetTexture({(*textures)["ThrusterExh"]});
	_exhaust->SetModels({(*models)["ThrusterExh"]});
	_exhaust->SetDrawEnabled(true);
	_exhaust->SetModelMatrix(glm::mat4(1.0));
	_exhaust->SetModelExternalMatrix(extMat);
	_exhaust->SetModelInnerMatrix(glm::mat4(1.0));
	_video->RegisterModel(_exhaust);
}

Thruster::~Thruster()
{
	_video->RemoveModel(_exhaust);
	delete _exhaust;
}

glm::vec3 Thruster::SetDirection(const glm::vec3& value)
{
	if (_angle < _targetAngle) {
		_angle += 0.05;
	} else if (_angle > _targetAngle) {
		_angle -= 0.05;
	}

	_angle = std::clamp(_angle, 0.0f, 90.0f);

	glm::mat4 baseMatrix = glm::translate(glm::mat4(1.0f), _position);
	baseMatrix = glm::rotate(
		baseMatrix,
		glm::radians(_angle),
		glm::vec3(-1, 0, 0));
	SetModelMatrix(baseMatrix);

	AdjustFAngle(value);
	AdjustRAngle(value);

	glm::mat4 exhaustMatrix = glm::translate(
		baseMatrix,
		glm::vec3(0, 2, 0));

	exhaustMatrix = glm::rotate(
		exhaustMatrix,
		glm::radians(_angleF),
		glm::vec3(1, 0, 0));

	_exhaust->SetModelMatrix(glm::rotate(
		exhaustMatrix,
		glm::radians(_angleR),
		glm::vec3(0, 0, 1)));

	glm::vec3 thrustDir(0, -1, 0);
	glm::mat4 rotMatrix = *GetModelExternalMatrix() *
		_exhaust->GetModelMatrix();
	thrustDir = rotMatrix * glm::vec4(thrustDir, 0.0f);

	float angleCos = glm::dot(
		glm::normalize(thrustDir),
		glm::normalize(value));

	if (angleCos > 0) {
		return glm::normalize(thrustDir) * angleCos;
	}

	return glm::vec3(0, 0, 0);
}

void Thruster::AdjustFAngle(const glm::vec3& value)
{
	glm::vec3 planePoint1(0, 0, 0);
	glm::vec3 planePoint2(0, 1, 0);
	glm::vec3 planePoint3(0, 1, 1);
	glm::vec3 normal(-1, 0, 0);

	glm::vec3 thrustDir(0, -1, 0);

	glm::mat4 matrix = *GetModelExternalMatrix() * GetModelMatrix();
	glm::mat4 rotMatrix = *GetModelExternalMatrix() *
		_exhaust->GetModelMatrix();

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

	if (_angle < 30) {
		_angleF = std::clamp(_angleF, -30.0f, 0.0f);
	} else {
		_angleF = std::clamp(_angleF, -30.0f, 30.0f);
	}
}

void Thruster::AdjustRAngle(const glm::vec3& value)
{
	if (_angle < 30) {
		if (_angleR > 0) {
			_angleR -= 0.1;
		} else if (_angleR < 0) {
			_angleR += 0.1;
		}

		return;
	}

	glm::vec3 planePoint1(0, 0, 0);
	glm::vec3 planePoint2(1, 0, 0);
	glm::vec3 planePoint3(1, 1, 0);
	glm::vec3 normal(0, 0, -1);

	glm::vec3 thrustDir(0, -1, 0);

	glm::mat4 matrix = *GetModelExternalMatrix() * GetModelMatrix();
	glm::mat4 rotMatrix = *GetModelExternalMatrix() *
		_exhaust->GetModelMatrix();

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
}

Wing::~Wing()
{
}

glm::vec3 Wing::SetSpeed(const glm::vec3& value)
{
	if (_angle < _targetAngle && _angleUp == 0) {
		_angle += 0.05;
	}
	
	if (_angle > _targetAngle) {
		_angle -= 0.05;
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

	SetModelMatrix(matrix);

	if (glm::length(value) < 0.00001) {
		return glm::vec3(0);
	}

	glm::vec3 forward = *GetModelExternalMatrix() * glm::vec4(0, -1, 0, 0);
	glm::vec3 right = *GetModelExternalMatrix() * glm::vec4(1, 0, 0, 0);
	glm::vec3 up = *GetModelExternalMatrix() * glm::vec4(0, 0, 1, 0);

	forward = glm::normalize(forward);
	right = glm::normalize(right);

	glm::vec3 effect = value;

	float angleCos = glm::dot(effect, forward);
	effect -= forward * angleCos;

	angleCos = glm::dot(effect, right);
	effect -= right * angleCos;

	effect -= up * glm::length(value) * 0.1f;

	float speedFactor = glm::length(value);

	speedFactor = std::clamp(speedFactor, 0.0f, 1.0f);

	return effect * speedFactor * (1.0f - _angleUp / 90.0f) * -0.5f;
}
