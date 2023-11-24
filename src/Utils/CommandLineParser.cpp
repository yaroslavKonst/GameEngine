#include "CommandLineParser.h"

#include <stdexcept>

#include "../Logger/logger.h"

CommandLineParser::Args CommandLineParser::Parse(
	int argc,
	char** argv,
	std::map<std::string, std::string> defaultKeys)
{
	Args result;

	for (auto& key : defaultKeys) {
		result.Keys.insert(key);
	}

	bool keyActive = false;
	std::string key;

	for (int idx = 0; idx < argc; ++idx) {
		std::string item(argv[idx]);

		if (keyActive) {
			result.Keys[key] = item;
			keyActive = false;
		} else {
			if (item.starts_with("--")) {
				keyActive = true;
				key = item.substr(2);
			} else {
				result.Args.insert(item);
			}
		}
	}

	if (keyActive) {
		throw std::runtime_error(
			"Last key in command line does not have value.");
	}

	return result;
}
