#include "ScriptHandler.h"

#include "../Utils/TextFileParser.h"
#include "../Utils/loader.h"

#include "../Logger/logger.h"

ScriptHandler::Scene ScriptHandler::LoadScene(std::string file, Video* video)
{
	TextFileParser::File script = TextFileParser::ParseFile(file);

	Scene scene;

	std::map<std::string, uint32_t> textureNames;

	for (auto& line : script) {
		if (line.size() == 0) {
			continue;
		}

		if (line[0] == "texture") {
			int texWidth;
			int texHeight;
			auto texData = Loader::LoadImage(
				line[2],
				texWidth,
				texHeight);

			uint32_t texId = video->GetTextures()->AddTexture(
				texWidth,
				texHeight,
				texData);

			textureNames[line[1]] = texId;
			scene.Textures.push_back(texId);
		} else if (line[0] == "object") {
			glm::vec3 pos(
				std::stof(line[4]),
				std::stof(line[5]),
				std::stof(line[6]));

			float rotX = std::stof(line[8]);
			float rotY = std::stof(line[9]);
			float rotZ = std::stof(line[7]);

			glm::mat4 matrix(1.0f);
			matrix = glm::translate(matrix, pos);

			matrix = glm::rotate(
				matrix,
				glm::radians(rotZ),
				glm::vec3(0, 0, 1));
			matrix = glm::rotate(
				matrix,
				glm::radians(rotX),
				glm::vec3(1, 0, 0));
			matrix = glm::rotate(
				matrix,
				glm::radians(rotY),
				glm::vec3(0, 1, 0));

			ExternModel* model = new ExternModel(
				line[1],
				textureNames[line[2]],
				textureNames[line[3]],
				matrix);

			if (line.size() == 11) {
				model->SetDrawLight(true);
				model->SetDrawLightMultiplier(
					std::stof(line[10]));
			}

			scene.Models.push_back(model);
		} else {
			Logger::Verbose() << "Unknown command " << line[0];

			throw std::runtime_error(
				"Unsupported command in scene script.");
		}
	}

	return scene;
}
