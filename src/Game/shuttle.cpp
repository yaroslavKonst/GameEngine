#include "shuttle.h"

#include <cstring>

#include "../Utils/loader.h"
#include "../Math/PlaneHelper.h"

#include "../Logger/logger.h"

static glm::dvec3 VecToGlm(const Math::Vec<3>& vec)
{
	return glm::dvec3(vec[0], vec[1], vec[2]);
}

static Math::Mat<4> GlmToMat(const glm::dmat4& mat)
{
	Math::Mat<4> res;

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			res[row][col] = mat[col][row];
		}
	}

	return res;
}

Shuttle::Shuttle(Common common, GravityField* gf)
{
	_common = common;
	_gf = gf;

	//_shipMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, 20));
	_shipMatrix = Math::Mat<4>(1.0);

	SetInputLayer(1);
	InputArea = {-1, -1, 1, 1};

	_common.video->Subscribe(this);

	_flightMode = false;
	_grounded = false;

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

	_thrusterPos = {
		{3.25, 11.07, 1.75},
		{-3.25, 11.07, 1.75},
		{4.65, 35, 1.75},
		{-4.65, 35, 1.75}
	};

	LoadAssets();
	BuildPhysicalFrame();

	_base = new Model;
	_base->TextureParams.SetAll(_textures["Base"]);
	_base->ModelParams.Model = _models["Base"];
	_base->DrawParams.Enabled = true;
	_base->ModelParams.Matrix = Math::Mat<4>(1.0);
	_base->ModelParams.ExternalMatrix = &_shipMatrix;
	_common.video->RegisterModel(_base);

	_roof = new Model;
	_roof->TextureParams.SetAll(_textures["Roof"]);
	_roof->ModelParams.Model = _models["Roof"];
	_roof->DrawParams.Enabled = true;
	_roof->ModelParams.Matrix = Math::Mat<4>(1.0);
	_roof->ModelParams.ExternalMatrix = &_shipMatrix;
	_common.video->RegisterModel(_roof);

	_thrusters.resize(4, nullptr);

	for (size_t i = 0; i < _thrusters.size(); ++i) {
		_thrusters[i] = new Thruster(
			_thrusterPos[i],
			_common.video,
			&_shipMatrix,
			&_textures,
			&_models);

		_thrusters[i]->TextureParams.SetAll(_textures["Thruster"]);
		_thrusters[i]->ModelParams.Model = _models["Thruster"];
		_thrusters[i]->DrawParams.Enabled = true;
		_thrusters[i]->ModelParams.Matrix = Math::Mat<4>(1.0);
		_thrusters[i]->ModelParams.ExternalMatrix = &_shipMatrix;
		_common.video->RegisterModel(_thrusters[i]);
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
	_wings[0]->ModelParams.Matrix = Math::Mat<4>(1.0);
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
	_wings[1]->ModelParams.Matrix = Math::Mat<4>(1.0);
	_wings[1]->ModelParams.ExternalMatrix = &_shipMatrix;
	_wings[1]->ModelParams.InnerMatrix = GlmToMat(glm::scale(
		glm::dmat4(1.0),
		glm::dvec3(-1, 1, 1)));
	_common.video->RegisterModel(_wings[1]);

	_cornerTextBox = new TextBox(_common.video, _common.textHandler);
	_cornerTextBox->SetPosition(-0.9, -0.8);
	_cornerTextBox->SetTextSize(0.05);
	_cornerTextBox->SetText("");
	_cornerTextBox->SetTextColor({1, 1, 1, 1});
	_cornerTextBox->SetDepth(0);
}

Shuttle::~Shuttle()
{
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

	RemovePhysicalFrame();
	UnloadAssets();
	_common.video->Unsubscribe(this);
}

void Shuttle::LoadAssets()
{
	auto td = Loader::LoadImage("Models/Shuttle/Base.png");
	_textures["Base"] = _common.video->LoadTexture(td);
	auto model = Loader::LoadModel("Models/Shuttle/Base.obj");
	_models["Base"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Roof.png");
	_textures["Roof"] = _common.video->LoadTexture(td);
	model = Loader::LoadModel("Models/Shuttle/Roof.obj");
	_models["Roof"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Thruster.png");
	_textures["Thruster"] = _common.video->LoadTexture(td);
	model = Loader::LoadModel("Models/Shuttle/Thruster.obj");
	_models["Thruster"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/ThrusterExh.png");
	_textures["ThrusterExh"] = _common.video->LoadTexture(td);
	model = Loader::LoadModel("Models/Shuttle/ThrusterExh.obj");
	_models["ThrusterExh"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Models/Shuttle/Wing.png");
	_textures["Wing"] = _common.video->LoadTexture(td);
	model = Loader::LoadModel("Models/Shuttle/Wing.obj");
	_models["Wing"] = _common.video->LoadModel(model);

	for (size_t index = 0; index < model.Indices.size(); index += 3) {
		uint32_t tmp = model.Indices[index + 1];
		model.Indices[index + 1] = model.Indices[index + 2];
		model.Indices[index + 2] = tmp;
	}

	_models["WingInverted"] = _common.video->LoadModel(model);

	td = Loader::LoadImage("Images/Circle.png");
	_textures["Point"] = _common.video->LoadTexture(td);
}

void Shuttle::UnloadAssets()
{
	for (auto& texture : _textures) {
		_common.video->UnloadTexture(texture.second);
	}

	for (auto& model : _models) {
		_common.video->UnloadModel(model.second);
	}
}

void Shuttle::BuildPhysicalFrame()
{
	_gearPos = {
		{0, 10, -3},
		{5, 30, -3},
		{-5, 30, -3}
	};

	SoftPhysicsValues::Vertex vertex;
	vertex.Mass = 5;
	vertex.Mu = 0.3;
	vertex.Bounciness = 0.1;

	std::vector<Math::Vec<3>> positions = {
		_gearPos[0],
		_gearPos[1],
		_gearPos[2],
		_thrusterPos[0],
		_thrusterPos[1],
		_thrusterPos[2],
		_thrusterPos[3]
	};

	_sprites.resize(positions.size());

	for (size_t idx = 0; idx < _sprites.size(); ++idx) {
		_sprites[idx] = new Sprite();
		_sprites[idx]->SpriteParams.Position = positions[idx];
		_sprites[idx]->SpriteParams.Up = {0, 0, 1};
		_sprites[idx]->SpriteParams.Size = {0.5, 0.5};
		_sprites[idx]->DrawParams.Enabled = true;
		_sprites[idx]->TextureParams.IsLight = true;
		_sprites[idx]->TextureParams.SetAll(_textures["Point"]);

		_common.video->RegisterSprite(_sprites[idx]);
	}

	for (Math::Vec<3> position : positions) {
		vertex.Position = position + Math::Vec<3>({0, 10, 7});
		SoftPhysicsParams.Vertices.push_back(vertex);
	}

	SoftObject::SoftPhysicsValues::Link link;
	link.K = 3000;
	link.Friction = 50;

	for (size_t index1 = 0; index1 < positions.size(); ++index1) {
		for (
			size_t index2 = index1 + 1;
			index2 < positions.size();
			++index2)
		{
			link.Index1 = index1;
			link.Index2 = index2;
			link.Length =
				(positions[index1] -
				positions[index2]).Length();

			SoftPhysicsParams.Links.push_back(link);
		}
	}

	_common.physicalEngine->RegisterObject(this);
}

void Shuttle::RemovePhysicalFrame()
{
	_common.physicalEngine->RemoveObject(this);

	for (Sprite* sprite : _sprites) {
		_common.video->RemoveSprite(sprite);
		delete sprite;
	}
}

void Shuttle::TickEarly()
{
	Math::Vec<3> locDirF({0, -1, 0});
	Math::Vec<3> locDirR({-1, 0, 0});
	Math::Vec<3> locDirU({0, 0, 1});

	Math::Vec<3> dirF = _shipMatrix * Math::Vec<4>(locDirF, 0.0);
	Math::Vec<3> dirR = _shipMatrix * Math::Vec<4>(locDirR, 0.0);
	Math::Vec<3> dirU = _shipMatrix * Math::Vec<4>(locDirU, 0.0);

	Math::Vec<3> currentSpeed(0.0);
	Math::Vec<3> targetSpeed(0.0);
	Math::Vec<3> position(0.0);
	double mass = 0;

	for (auto& vertex : SoftPhysicsParams.Vertices) {
		currentSpeed += vertex.Speed;
		mass += vertex.Mass;
		position += vertex.Position;
	}

	currentSpeed /= SoftPhysicsParams.Vertices.size();
	position /= SoftPhysicsParams.Vertices.size();

	if (_flightMode) {
		targetSpeed =
			dirF * _controlF +
			dirR * _controlR +
			dirU * _controlU;

		targetSpeed *= 10.0;

		if (_fastMode) {
			targetSpeed += dirF * _controlF * 1000.0;
		}
	} else {
		targetSpeed = {0, 0, 0};

		if (_fastMode) {
			targetSpeed = dirF * 100.0;
		}
	}

	_wings[0]->SetAngle(currentSpeed.Length() / 10.0);
	_wings[1]->SetAngle(currentSpeed.Length() / 10.0);

	Math::Vec<3> targetForce = (targetSpeed - currentSpeed) * mass;

	Math::Vec<3> mg = (*_gf)(position) * mass / 7.0;
	Math::Vec<3> envForce = -currentSpeed * 0.01;

	targetForce -= mg;
	targetForce -= envForce;

	Math::Vec<3> wingForce = GetWingForce(currentSpeed, targetForce);
	targetForce -= wingForce;

	if (/*_grounded &&*/ !_flightMode) {
		targetForce = {0, 0, 0};
	}

	Math::Vec<3> force =
		mg +
		//envForce +
		wingForce;

	SoftPhysicsParams.Force = force;

	SetThrusterForce(
		targetForce,
		currentSpeed,
		dirF,
		dirR,
		dirU);

	SoftPhysicsParams.Vertices[5].Force += envForce / 2.0;
	SoftPhysicsParams.Vertices[6].Force += envForce / 2.0;

	Math::Vec<3> yawForce = dirR * _controlYaw *
		currentSpeed.Length() * 0.05;
	SoftPhysicsParams.Vertices[5].Force -= yawForce;
	SoftPhysicsParams.Vertices[6].Force -= yawForce;

	Math::Vec<3> pitchForce = dirU * _controlPitch *
		currentSpeed.Length() * 0.05;
	SoftPhysicsParams.Vertices[5].Force -= pitchForce;
	SoftPhysicsParams.Vertices[6].Force -= pitchForce;

	Math::Vec<3> rollForce = dirU * _controlRoll *
		currentSpeed.Length() * 0.05;
	SoftPhysicsParams.Vertices[5].Force += rollForce;
	SoftPhysicsParams.Vertices[6].Force -= rollForce;

	if (_flightMode) {
		Math::Vec<3> cameraPosition = position +
			dirU * 10.0 * _cameraDist - dirF * 30.0 * _cameraDist;
		Math::Vec<3> cameraTarget = position;

		_common.video->SetCameraPosition(cameraPosition);
		_common.video->SetCameraTarget(cameraTarget);
		_common.video->SetCameraUp(dirU);

		_cornerTextBox->SetText(
			_common.localizer->Localize("FlightData:\nSpeed: ") +
			std::to_string(currentSpeed[0]) + " " +
			std::to_string(currentSpeed[1]) + " " +
			std::to_string(currentSpeed[2]) + "\n" +

			_common.localizer->Localize("Target speed: ") +
			std::to_string(targetSpeed[0]) + " " +
			std::to_string(targetSpeed[1]) + " " +
			std::to_string(targetSpeed[2]) + "\n" +

			_common.localizer->Localize("Flaps: ") +
			std::to_string(_wings[0]->GetFlap()) + "\n" +

			_common.localizer->Localize("F angle: ") +
			std::to_string(_thrusters[0]->GetFAngle()));
		_cornerTextBox->Activate();
	}
}

void Shuttle::Tick()
{
	_controlYaw *= 0.97;
	_controlPitch *= 0.97;
	_controlRoll *= 0.97;

	// TODO:
	//_shipMatrix = _shipMatrix;

	Math::Vec<3> forward =
		(SoftPhysicsParams.Vertices[3].Position -
		SoftPhysicsParams.Vertices[5].Position +
		SoftPhysicsParams.Vertices[4].Position -
		SoftPhysicsParams.Vertices[6].Position).Normalize();

	Math::Vec<3> right =
		(SoftPhysicsParams.Vertices[3].Position -
		SoftPhysicsParams.Vertices[4].Position +
		SoftPhysicsParams.Vertices[5].Position -
		SoftPhysicsParams.Vertices[6].Position).Normalize();

	Math::Vec<3> x = right;
	Math::Vec<3> y = -forward;
	Math::Vec<3> z = x.Cross(y).Normalize();

	y = z.Cross(x);

	Math::Mat<4> cvtMat(1.0);

	cvtMat[0][0] = x[0];
	cvtMat[1][0] = x[1];
	cvtMat[2][0] = x[2];

	cvtMat[0][1] = y[0];
	cvtMat[1][1] = y[1];
	cvtMat[2][1] = y[2];

	cvtMat[0][2] = z[0];
	cvtMat[1][2] = z[1];
	cvtMat[2][2] = z[2];

	Math::Vec<3> position =
		(SoftPhysicsParams.Vertices[3].Position +
		SoftPhysicsParams.Vertices[4].Position) / 2.0;

	position += forward * 11.07 - z * 1.75;

	cvtMat[0][3] = position[0];
	cvtMat[1][3] = position[1];
	cvtMat[2][3] = position[2];

	_shipMatrix = cvtMat;

	for (size_t i = 0; i < _sprites.size(); ++i) {
		_sprites[i]->SpriteParams.Position =
			SoftPhysicsParams.Vertices[i].Position;
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
			_controlF = 0;
			_controlR = 0;
			_controlU = 0;
			_controlYaw = 0;
			_controlPitch = 0;
			_controlRoll = 0;

			SetInputEnabled(false);
			_cornerTextBox->Deactivate();
			_flightMode = false;
			return;
		}
	}
}

void Shuttle::SetThrusterForce(
	const Math::Vec<3>& force,
	const Math::Vec<3>& speed,
	const Math::Vec<3>& dirF,
	const Math::Vec<3>& dirR,
	const Math::Vec<3>& dirU)
{
	Math::Vec<3> result(0.0);

	for (size_t i = 0; i < _thrusters.size(); ++i) {
		Math::Vec<3> forceMod(0.0);

		if (i < 2) {
			forceMod += dirR * _controlYaw;
		} else {
			forceMod -= dirR * _controlYaw;
		}

		if (i % 2 == 0) {
			forceMod += dirF * _controlYaw;
		} else {
			forceMod -= dirF * _controlYaw;
		}

		if (i < 2) {
			forceMod += dirU * _controlPitch;
		} else {
			forceMod -= dirU * _controlPitch;
		}

		if (i % 2 == 0) {
			forceMod += dirU * _controlRoll;
		} else {
			forceMod -= dirU * _controlRoll;
		}

		Math::Vec<3> res = _thrusters[i]->SetDirection(
			force / 4.0 + forceMod,
			speed);

		result += res;

		if (res.Length() > 0.000001) {
			res = res.Normalize();
			forceMod = res * res.Dot(forceMod);
		} else {
			forceMod = 0.0;
		}

		SoftPhysicsParams.Vertices[i + 3].Force = forceMod;
	}

	SoftPhysicsParams.Force += result;
}

Math::Vec<3> Shuttle::GetWingForce(
	const Math::Vec<3>& speed,
	const Math::Vec<3>& force)
{
	Math::Vec<3> wingForce =
		_wings[0]->SetSpeed(speed, force) +
		_wings[1]->SetSpeed(speed, force);

	return wingForce;
}

Thruster::Thruster(
	const Math::Vec<3>& position,
	Video* video,
	Math::Mat<4>* extMat,
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
	_exhaust->ModelParams.Matrix = Math::Mat<4>(1.0);
	_exhaust->ModelParams.ExternalMatrix = extMat;
	_video->RegisterModel(_exhaust);
}

Thruster::~Thruster()
{
	_video->RemoveModel(_exhaust);
	delete _exhaust;
}

Math::Vec<3> Thruster::SetDirection(
	const Math::Vec<3>& value,
	const Math::Vec<3>& speed)
{
	if (_angle < _targetAngle) {
		_angle += 0.05;
	} else if (_angle > _targetAngle) {
		_angle -= 0.05;
	}

	_angle = std::clamp(_angle, 0.0, 120.0);

	glm::dmat4 baseMatrix = glm::translate(
		glm::dmat4(1.0),
		VecToGlm(_position));
	baseMatrix = glm::rotate(
		baseMatrix,
		glm::radians(_angle),
		glm::dvec3(-1, 0, 0));
	ModelParams.Matrix = GlmToMat(baseMatrix);

	AdjustFAngle(value, speed);
	AdjustRAngle(value, speed);

	glm::dmat4 exhaustMatrix = glm::translate(
		baseMatrix,
		glm::dvec3(0, 2, 0));

	exhaustMatrix = glm::rotate(
		exhaustMatrix,
		glm::radians(_angleF),
		glm::dvec3(1, 0, 0));

	_exhaust->ModelParams.Matrix = GlmToMat(glm::rotate(
		exhaustMatrix,
		glm::radians(_angleR),
		glm::dvec3(0, 0, 1)));

	Math::Vec<3> thrustDir({0, -1, 0});
	Math::Mat<4> rotMatrix = *ModelParams.ExternalMatrix *
		_exhaust->ModelParams.Matrix;
	thrustDir = rotMatrix * Math::Vec<4>(thrustDir, 0.0);

	double angleCos = thrustDir.Normalize().Dot(value.Normalize());

	if (angleCos > 0) {
		return thrustDir.Normalize() * angleCos *
			std::min<double>(5000.0, value.Length());
	}

	return Math::Vec<3>(0.0);
}

void Thruster::AdjustFAngle(
	const Math::Vec<3>& value,
	const Math::Vec<3>& speed)
{
	if (_angle < 30 && speed.Length() > 30.0) {
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

	Math::Vec<3> planePoint1({0, 0, 0});
	Math::Vec<3> planePoint2({0, 1, 0});
	Math::Vec<3> planePoint3({0, 1, 1});
	Math::Vec<3> normal({-1, 0, 0});

	Math::Vec<3> thrustDir({0, -1, 0});

	Math::Mat<4> matrix = *ModelParams.ExternalMatrix * ModelParams.Matrix;
	Math::Mat<4> rotMatrix = *ModelParams.ExternalMatrix *
		_exhaust->ModelParams.Matrix;

	planePoint1 = matrix * Math::Vec<4>(planePoint1, 1.0);
	planePoint2 = matrix * Math::Vec<4>(planePoint2, 1.0);
	planePoint3 = matrix * Math::Vec<4>(planePoint3, 1.0);
	normal = matrix * Math::Vec<4>(normal, 0.0);
	thrustDir = rotMatrix * Math::Vec<4>(thrustDir, 0.0);

	PlaneHelper::Plane enginePlane =
		PlaneHelper::PlaneByThreePoints(
			planePoint1,
			planePoint2,
			planePoint3);

	enginePlane[3] = 0;

	Math::Vec<3> valueDir =
		PlaneHelper::ProjectPointToPlane(value, enginePlane);
	thrustDir =
		PlaneHelper::ProjectPointToPlane(thrustDir, enginePlane);

	valueDir = valueDir.Normalize();
	thrustDir = thrustDir.Normalize();

	Math::Vec<3> spinDir = valueDir.Cross(thrustDir);
	double dot = valueDir.Dot(thrustDir);

	double crossDot = spinDir.Dot(normal.Normalize());

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

	_angleF = std::clamp(_angleF, -30.0, 30.0);
}

void Thruster::AdjustRAngle(
	const Math::Vec<3>& value,
	const Math::Vec<3>& speed)
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

	Math::Vec<3> planePoint1({0, 0, 0});
	Math::Vec<3> planePoint2({1, 0, 0});
	Math::Vec<3> planePoint3({1, 1, 0});
	Math::Vec<3> normal({0, 0, -1});

	Math::Vec<3> thrustDir({0, -1, 0});

	Math::Mat<4> matrix = *ModelParams.ExternalMatrix * ModelParams.Matrix;
	Math::Mat<4> rotMatrix = *ModelParams.ExternalMatrix *
		_exhaust->ModelParams.Matrix;

	planePoint1 = matrix * Math::Vec<4>(planePoint1, 1.0);
	planePoint2 = matrix * Math::Vec<4>(planePoint2, 1.0);
	planePoint3 = matrix * Math::Vec<4>(planePoint3, 1.0);
	normal = matrix * Math::Vec<4>(normal, 0.0);
	thrustDir = rotMatrix * Math::Vec<4>(thrustDir, 0.0);

	PlaneHelper::Plane enginePlane =
		PlaneHelper::PlaneByThreePoints(
			planePoint1,
			planePoint2,
			planePoint3);

	enginePlane[3] = 0;

	Math::Vec<3> valueDir =
		PlaneHelper::ProjectPointToPlane(value, enginePlane);
	thrustDir =
		PlaneHelper::ProjectPointToPlane(thrustDir, enginePlane);

	valueDir = valueDir.Normalize();
	thrustDir = thrustDir.Normalize();

	Math::Vec<3> spinDir = valueDir.Cross(thrustDir);
	double dot = valueDir.Dot(thrustDir);

	double crossDot = spinDir.Dot(normal.Normalize());

	if (crossDot > 0) {
		if (dot < 0 || abs(crossDot) > 0.2) {
			_angleR += 1;
		} else {
			_angleR += 0.1;
		}
	} else if (crossDot < 0) {
		if (dot < 0 || abs(crossDot) > 0.2) {
			_angleR -= 1;
		} else {
			_angleR -= 0.1;
		}
	}

	_angleR = std::clamp(_angleR, -30.0, 30.0);
}

Wing::Wing(
	const Math::Vec<3>& position,
	Video* video,
	Math::Mat<4>* extMat,
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

void Wing::IncFlap(double value)
{
	_targetFlap += value;
	_targetFlap = std::clamp(_targetFlap, 0.0, 1.0);
}

static Math::Vec<3> ProjVector(
	const Math::Vec<3>& projectedVector,
	const Math::Vec<3>& onProjVector)
{
	return onProjVector *
		projectedVector.Dot(onProjVector) /
		onProjVector.Dot(onProjVector);
}

Math::Vec<3> Wing::SetSpeed(
	const Math::Vec<3>& value,
	const Math::Vec<3>& force)
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

	_angle = std::clamp(_angle, 0.0, 30.0);

	if (_angleUp < _targetAngleUp && _angle < 1) {
		_angleUp += 0.05;
	}
	
	if (_angleUp > _targetAngleUp) {
		_angleUp -= 0.05;
	}

	_angleUp = std::clamp(_angleUp, 0.0, 90.0);

	glm::dmat4 matrix = glm::translate(
		glm::dmat4(1.0),
		VecToGlm(_position));

	matrix = glm::rotate(
		matrix,
		glm::radians(_angle),
		glm::dvec3(0, 0, _inverted ? -1 : 1));
	matrix = glm::rotate(
		matrix,
		glm::radians(_angleUp),
		glm::dvec3(0, _inverted ? 1 : -1, 0));

	ModelParams.Matrix = GlmToMat(matrix);

	if (value.Length() < 0.00001) {
		return Math::Vec<3>(0.0);
	}

	Math::Vec<3> forward =
		*ModelParams.ExternalMatrix * Math::Vec<4>({0, -1, 0, 0});
	Math::Vec<3> right = *ModelParams.ExternalMatrix *
		Math::Vec<4>({-1, 0, 0, 0});
	Math::Vec<3> up = *ModelParams.ExternalMatrix *
		Math::Vec<4>({0, 0, 1, 0});

	forward = forward.Normalize();
	right = right.Normalize();
	up = up.Normalize();

	Math::Vec<3> speedProj = value - ProjVector(value, forward);
	Math::Vec<3> forceProj = force - ProjVector(force, forward);

	Math::Vec<3> effect = -speedProj.Normalize();
	effect += up * _flap * 0.3;

	double speedFactor = value.Length() * 2.0;

	effect *= speedFactor * (1.0 - _angleUp / 90.0);

	double forceLenDiff = effect.Length() - forceProj.Length();
	bool forceUp = up.Dot(forceProj) > 0;

	if (forceLenDiff > 0 || !forceUp) {
		IncFlap(-0.01);
	} else if (forceLenDiff < 0) {
		IncFlap(0.01);
	}

	return effect;
}
