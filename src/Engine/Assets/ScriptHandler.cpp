#include "ScriptHandler.h"

#include "../Utils/TextFileParser.h"
#include "../Utils/loader.h"

#include "../Logger/logger.h"

Animation ScriptHandler::LoadAnimation(std::string file)
{
	TextFileParser::File script = TextFileParser::ParseFile(file);

	Animation animation;

	std::vector<float> timeValues;
	std::vector<Animation::TimePoint> timePoints;

	for (auto& line : script) {
		if (line.size() == 0) {
			continue;
		}

		if (line[0] == "point") {
			float x = std::stof(line[1]);
			float y = std::stof(line[2]);
			float z = std::stof(line[3]);

			float rx = std::stof(line[5]);
			float ry = std::stof(line[6]);
			float rz = std::stof(line[4]);

			float time = std::stof(line[7]);

			timeValues.push_back(time);
			timePoints.push_back({{x, y, z}, rx, ry, rz});
		} else if (line[0] == "cycle") {
			if (line[1] == "0") {
				animation.SetCycle(false);
			} else if (line[1] == "1") {
				animation.SetCycle(true);
			} else {
				throw std::runtime_error(
					"Invalid 'cycle' value.");
			}
		} else if (line[0] == "positions") {
			if (line[1] == "1") {
				animation.SetTwoPos(false);
			} else if (line[1] == "2") {
				animation.SetTwoPos(true);
			} else {
				throw std::runtime_error(
					"Invalid 'positions' value.");
			}
		} else {
			throw std::runtime_error(
				"Invalid animation script command.");
		}
	}

	animation.SetTimePoints(timePoints);
	animation.SetTimeValues(timeValues);

	return animation;
}
