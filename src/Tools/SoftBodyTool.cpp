#include <algorithm>
#include <unistd.h>

#include "../Engine/Video/video.h"
#include "../Engine/Video/GUI/button.h"
#include "../Engine/Logger/logger.h"
#include "../Engine/Assets/localizer.h"
#include "../Engine/Assets/package.h"

#define INTERPOLATE_GROUP 10000

template<class T, class E>
bool Contains(const T& container, const E& item)
{
	return container.find(item) != container.end();
}

template<int Dim>
class CompVec : public Math::Vec<Dim>
{
public:
	CompVec()
	{ }

	CompVec(const Math::Vec<Dim>& vec) : Math::Vec<Dim>(vec)
	{ }

	bool operator<(const CompVec<Dim>& vec) const
	{
		for (int i = 0; i < Dim; ++i) {
			if ((*this)[i] != vec[i]) {
				return (*this)[i] < vec[i];
			}
		}

		return false;
	}
};

class App : public InputHandler
{
public:
	App(Video* video, std::string modelName, std::string saveName)
	{
		_video = video;

		_work = true;

		_cameraDist = 10;
		_angleH = M_PI / 4;
		_angleV = M_PI / 4;
		_rotationActive = false;

		_saveName = saveName;

		_video->SetFOV(80);
		_video->SetCameraUp({0, 0, 1});

		_localizer = new Localizer("Locale/en");

		auto glyphs = Text::LoadFont(
			"Fonts/DroidSans.ttf",
			_localizer->GetCharSet());

		_textHandler = new TextHandler(_video, glyphs);

		_vertexData = Loader::LoadModel(modelName);
		_model.ModelParams.Model = _video->LoadModel(_vertexData);

		Loader::Image texImage;
		texImage.Width = 1;
		texImage.Height = 1;
		texImage.PixelData = {255, 255, 255, 255};
		_model.TextureParams.SetAll(_video->LoadTexture(texImage));

		_model.DrawParams.Enabled = true;
		_model.ModelParams.Matrix = Math::Mat<4>(1.0);

		_video->RegisterModel(&_model);

		std::set<CompVec<3>> uniqueVerticesSet;

		for (auto& vertex : _vertexData.Vertices) {
			if (!Contains(uniqueVerticesSet, vertex)) {
				uniqueVerticesSet.insert(vertex);
				_uniqueVertices.push_back(vertex);
			}
		}

		_nextColor = {0, 0, 0, 1};
		_vertexGroups.resize(_uniqueVertices.size(), {0, 0});
		_interpolateGroups.resize(_uniqueVertices.size(), 0);
		_setGroupMode = 0;
		_currentVertex = 0;
		_groupColors[INTERPOLATE_GROUP] = {1, 1, 1, 1};

		_vertexSprites.resize(_uniqueVertices.size(), nullptr);
		_vertexSprites[0] = new Sprite();
		_vertexSprites[0]->SpriteParams.Position =
			_uniqueVertices[0];
		_vertexSprites[0]->SpriteParams.Size = {0.1, 0.1};
		_vertexSprites[0]->SpriteParams.Offset = 0.1;
		_vertexSprites[0]->SpriteParams.Up = {0, 0, 1};
		_vertexSprites[0]->TextureParams.SetAll(
			_model.TextureParams.Diffuse);
		_vertexSprites[0]->DrawParams.Enabled = true;
		_vertexSprites[0]->DrawParams.ColorMultiplier = {1, 0, 0, 1};
		_vertexSprites[0]->TextureParams.IsLight = true;

		_video->RegisterSprite(_vertexSprites[0]);

		_video->SetCameraPosition({
			_cameraDist,
			_cameraDist,
			_cameraDist
		});
		_video->SetCameraTarget({0, 0, 0});

		_light.Type = Light::Type::Point;
		_light.Color = {10, 10, 10};
		_light.Position = {10, 10, 10};
		_light.Enabled = true;
		_video->RegisterLight(&_light);

		SetInputEnabled(true);
		_video->Subscribe(this);
	}

	~App()
	{
		_video->Unsubscribe(this);

		for (Sprite* s : _vertexSprites) {
			if (s) {
				_video->RemoveSprite(s);
				delete s;
			}
		}

		_video->RemoveLight(&_light);
		_video->RemoveModel(&_model);

		delete _textHandler;
		delete _localizer;
	}

	void MainLoop()
	{
		Logger::Verbose() << "App started.";

		while (_work) {
			usleep(20000);

			Math::Vec<3> pos = {
				_cameraDist * cos(_angleH) * cos(_angleV),
				_cameraDist * sin(_angleH) * cos(_angleV),
				_cameraDist * sin(_angleV)
			};

			_light.Color = {_cameraDist, _cameraDist, _cameraDist};

			_light.Position = pos;
			_video->SetCameraPosition(pos);
			_video->SetCameraTarget({0, 0, 0});

			_video->SubmitScene();
		}

		Logger::Verbose() << "Saving model.";

		SaveModel();

		Logger::Verbose() << "App stopped.";
	}

	void Stop()
	{
		_work = false;
	}

	bool InInputArea(float x, float y) override
	{
		return true;
	}

	bool Key(int key, int scancode, int action, int mods) override
	{
		if (action != GLFW_PRESS) {
			return false;
		}

		if (key == GLFW_KEY_N) {
			if (_setGroupMode) {
				Logger::Warning() <<
					"Set group " << _setGroupMode;
				return false;
			}

			++_currentVertex;

			if (_currentVertex >= _vertexGroups.size()) {
				Logger::Warning() << "Last vertex processed.";
				--_currentVertex;
				return false;
			}

			if (!_vertexSprites[_currentVertex]) {
				_vertexSprites[_currentVertex] = new Sprite();
				_video->RegisterSprite(
					_vertexSprites[_currentVertex]);
			}

			_vertexSprites[_currentVertex]->SpriteParams.Position =
				_uniqueVertices[_currentVertex];
			_vertexSprites[_currentVertex]->SpriteParams.Size =
				{0.1, 0.1};
			_vertexSprites[_currentVertex]->SpriteParams.Up =
				{0, 0, 1};
			_vertexSprites[_currentVertex]->SpriteParams.Offset =
				0.11;
			_vertexSprites[_currentVertex]->TextureParams.SetAll(
				_model.TextureParams.Diffuse);
			_vertexSprites[_currentVertex]->DrawParams.Enabled =
				true;
			_vertexSprites[_currentVertex]->DrawParams.
				ColorMultiplier = {1, 0, 0, 1};
			_vertexSprites[_currentVertex]->TextureParams.IsLight =
				true;
		} else if (key == GLFW_KEY_P) {
			if (_setGroupMode) {
				Logger::Warning() <<
					"Set group " << _setGroupMode;
				return false;
			}

			if (_currentVertex == 0) {
				Logger::Warning() << "First vertex is reached.";
				return false;
			}

			--_currentVertex;

			if (!_vertexSprites[_currentVertex]) {
				_vertexSprites[_currentVertex] = new Sprite();
				_video->RegisterSprite(
					_vertexSprites[_currentVertex]);
			}

			_vertexSprites[_currentVertex]->SpriteParams.Position =
				_uniqueVertices[_currentVertex];
			_vertexSprites[_currentVertex]->SpriteParams.Size =
				{0.1, 0.1};
			_vertexSprites[_currentVertex]->SpriteParams.Up =
				{0, 0, 1};
			_vertexSprites[_currentVertex]->SpriteParams.Offset =
				0.11;
			_vertexSprites[_currentVertex]->TextureParams.SetAll(
				_model.TextureParams.Diffuse);
			_vertexSprites[_currentVertex]->DrawParams.Enabled =
				true;
			_vertexSprites[_currentVertex]->DrawParams.
				ColorMultiplier = {1, 0, 0, 1};
			_vertexSprites[_currentVertex]->TextureParams.IsLight =
				true;
		} else if (key == GLFW_KEY_0) {
			SetVertexGroup(_currentVertex, 0);
		} else if (key == GLFW_KEY_1) {
			SetVertexGroup(_currentVertex, 1);
		} else if (key == GLFW_KEY_2) {
			SetVertexGroup(_currentVertex, 2);
		} else if (key == GLFW_KEY_3) {
			SetVertexGroup(_currentVertex, 3);
		} else if (key == GLFW_KEY_4) {
			SetVertexGroup(_currentVertex, 4);
		} else if (key == GLFW_KEY_5) {
			SetVertexGroup(_currentVertex, 5);
		} else if (key == GLFW_KEY_6) {
			SetVertexGroup(_currentVertex, 6);
		} else if (key == GLFW_KEY_7) {
			SetVertexGroup(_currentVertex, 7);
		} else if (key == GLFW_KEY_8) {
			SetVertexGroup(_currentVertex, 8);
		} else if (key == GLFW_KEY_9) {
			SetVertexGroup(_currentVertex, 9);
		} else if (key == GLFW_KEY_A) {
			SetVertexGroup(_currentVertex, 10);
		} else if (key == GLFW_KEY_B) {
			SetVertexGroup(_currentVertex, 11);
		} else if (key == GLFW_KEY_C) {
			SetVertexGroup(_currentVertex, 12);
		} else if (key == GLFW_KEY_D) {
			SetVertexGroup(_currentVertex, 13);
		} else if (key == GLFW_KEY_E) {
			SetVertexGroup(_currentVertex, 14);
		} else if (key == GLFW_KEY_F) {
			SetVertexGroup(_currentVertex, 15);
		} else if (key == GLFW_KEY_I) {
			SetVertexGroup(_currentVertex, INTERPOLATE_GROUP);
		}

		return false;
	}

	bool MouseButton(int button, int action, int mods) override
	{
		if (button != GLFW_MOUSE_BUTTON_LEFT) {
			return false;
		}

		if (action == GLFW_PRESS) {
			if (_rotationActive) {
				return true;
			}

			_rotationActive = true;
			_video->ToggleRawMouseInput();
		} else if (action == GLFW_RELEASE) {
			if (!_rotationActive) {
				return true;
			}

			_rotationActive = false;
			_video->ToggleRawMouseInput();
		}

		return true;
	}

	bool Scroll(double xoffset, double yoffset) override
	{
		_cameraDist += yoffset / 2;
		return true;
	}

	bool MouseMoveRaw(double xoffset, double yoffset) override
	{
		if (_rotationActive) {
			_angleH -= xoffset / 100;
			_angleV += yoffset / 100;

			if (_angleH >= M_PI * 2.0) {
				_angleH -= M_PI * 2.0;
			} else if (_angleH < 0) {
				_angleH += M_PI * 2.0;
			}

			_angleV = std::clamp(_angleV, -M_PI / 2.1, M_PI / 2.1);
		}

		return true;
	}

	bool WindowClose() override
	{
		_work = false;
		return true;
	}

private:
	volatile bool _work;

	Video* _video;
	Localizer* _localizer;
	TextHandler* _textHandler;

	std::string _saveName;

	Loader::VertexData _vertexData;
	Model _model;
	bool _modelRotation;

	size_t _currentVertex;
	std::vector<Math::Vec<3>> _uniqueVertices;
	std::vector<Sprite*> _vertexSprites;
	std::vector<std::pair<int, int>> _vertexGroups;
	std::vector<int> _interpolateGroups;
	Math::Vec<4> _nextColor;
	std::map<int, Math::Vec<4>> _groupColors;

	double _cameraDist;
	double _angleH;
	double _angleV;
	bool _rotationActive;

	Light _light;

	int _setGroupMode;

	void SetVertexGroup(int vertex, int group)
	{
		if (_setGroupMode == 1) {
			Logger::Warning() << "Set group 2";
			_vertexGroups[vertex].first = group;
			_setGroupMode = 2;
			return;
		}

		if (_setGroupMode == 2) {
			Logger::Warning() << "Set group done";
			_vertexGroups[vertex].second = group;
			_setGroupMode = 0;
			return;
		}

		_vertexGroups[vertex].first = group;

		if (group == INTERPOLATE_GROUP) {
			Logger::Warning() << "Set group 1";
			_setGroupMode = 1;
			_interpolateGroups[vertex] = 1;
		} else {
			_interpolateGroups[vertex] = 0;
		}

		Math::Vec<4> color;

		if (_groupColors.find(group) == _groupColors.end()) {
			color = _nextColor;
			_groupColors[group] = _nextColor;
			_nextColor[1] += 0.2;

			if (_nextColor[1] > 1) {
				_nextColor[2] += 0.2;
				_nextColor[1] = 0;
			}
		} else {
			color = _groupColors[group];
		}

		for (int i = 0; i < 4; ++i) {
			_vertexSprites[vertex]->DrawParams.ColorMultiplier[i] =
				color[i];
		}

		_vertexSprites[vertex]->SpriteParams.Offset = 0.1;
	}

	template<typename T>
	void Swap(T& a, T& b)
	{
		T tmp = a;
		a = b;
		b = tmp;
	}

	void SaveModel()
	{
		bool sorted = false;

		while (!sorted) {
			sorted = true;

			for (size_t i = 1; i < _vertexGroups.size(); ++i) {
				bool wrongOrder = false;

				if (
					_interpolateGroups[i - 1] &&
					!_interpolateGroups[i])
				{
					wrongOrder = true;
				} else if (
					!_interpolateGroups[i - 1] &&
					!_interpolateGroups[i])
				{
					wrongOrder =
						_vertexGroups[i - 1].first >
						_vertexGroups[i].first;
				}

				if (wrongOrder) {
					sorted = false;

					Swap(
						_vertexGroups[i],
						_vertexGroups[i - 1]);
					Swap(
						_uniqueVertices[i],
						_uniqueVertices[i - 1]);
					Swap(
						_interpolateGroups[i],
						_interpolateGroups[i - 1]);
				}
			}
		}

		std::map<CompVec<3>, uint32_t> vertexIndices;
		std::map<CompVec<3>, uint32_t> normalIndices;
		std::map<CompVec<2>, uint32_t> textureIndices;

		uint32_t index = 1;

		for (auto& vertex : _uniqueVertices) {
			if (!Contains(vertexIndices, vertex)) {
				vertexIndices[vertex] = index;
				++index;
			}
		}

		std::fstream outFile;
		outFile.open(_saveName, std::ios::out);

		int currentGroup = -1;

		for (size_t i = 0; i < _uniqueVertices.size(); ++i) {
			if (
				currentGroup != _vertexGroups[i].first ||
				_interpolateGroups[i])
			{
				currentGroup = _vertexGroups[i].first;

				if (!_interpolateGroups[i]) {
					outFile << "vg " << currentGroup <<
						std::endl;
				} else {
					currentGroup = -1;
					outFile << "vg %Interpolate ";

					int group[2] = {
						_vertexGroups[i].first,
						_vertexGroups[i].second
					};

					double distance[2] = {1e+40, 1e+40};

					for (
						size_t idx = 0;
						idx < _uniqueVertices.size();
						++idx)
					{
						if (_interpolateGroups[idx])
						{
							continue;
						}

						double dist =
							(_uniqueVertices[idx] -
							_uniqueVertices[i]).
							Length();

						for (int gi = 0; gi < 2; ++gi) {
							bool valid =
								dist <
								distance[gi] &&
								group[gi] ==
								_vertexGroups[idx].first;

							if (valid) {
								distance[gi] =
									dist;
							}
						}
					}

					outFile << group[0] << " " << group[1];

					float coeff = distance[0] /
						(distance[0] + distance[1]);

					outFile << " " << coeff << std::endl;
				}
			}

			outFile << "v " <<
				_uniqueVertices[i][0] << " " <<
				_uniqueVertices[i][1] << " " <<
				_uniqueVertices[i][2] << std::endl;
		}

		index = 1;

		for (auto& normal : _vertexData.Normals) {
			if (!Contains(normalIndices, normal)) {
				normalIndices[normal] = index;
				++index;

				outFile << "vn " <<
					normal[0] << " " <<
					normal[1] << " " <<
					normal[2] << std::endl;
			}
		}

		index = 1;

		for (auto& tex : _vertexData.TexCoords) {
			if (!Contains(textureIndices, tex)) {
				textureIndices[tex] = index;
				++index;

				outFile << "vt " <<
					tex[0] << " " <<
					1.0 - tex[1] << std::endl;
			}
		}

		for (uint32_t i = 0; i < _vertexData.Indices.size(); i += 3) {
			// v/t/n
			outFile << "f ";

			uint32_t i1 = _vertexData.Indices[i];
			uint32_t i2 = _vertexData.Indices[i + 1];
			uint32_t i3 = _vertexData.Indices[i + 2];

			outFile << vertexIndices[_vertexData.Vertices[i1]];
			outFile << '/';
			outFile << textureIndices[_vertexData.TexCoords[i1]];
			outFile << '/';
			outFile << normalIndices[_vertexData.Normals[i1]];
			outFile << ' ';

			outFile << vertexIndices[_vertexData.Vertices[i2]];
			outFile << '/';
			outFile << textureIndices[_vertexData.TexCoords[i2]];
			outFile << '/';
			outFile << normalIndices[_vertexData.Normals[i2]];
			outFile << ' ';

			outFile << vertexIndices[_vertexData.Vertices[i3]];
			outFile << '/';
			outFile << textureIndices[_vertexData.TexCoords[i3]];
			outFile << '/';
			outFile << normalIndices[_vertexData.Normals[i3]];
			outFile << std::endl;
		}

		outFile.close();
	}
};

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Warning);

	if (argc < 3) {
		Logger::Error() << "Usage: <input file> <output file>";
		return 1;
	}

	Package::LoadPackage("resources.bin");

	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 1;

	Video* video = new Video(
		800,
		800,
		"Soft Body Tool",
		"Soft Body Tool",
		&videoSettings);

	App* app = new App(video, argv[1], argv[2]);

	std::thread* videoThread =
		new std::thread([video]() {video->MainLoop();});

	app->MainLoop();

	video->Stop();
	videoThread->join();
	delete videoThread;

	delete app;
	delete video;

	Package::UnloadPackage();

	return 0;
}
