#include "localizer.h"

#include <stdexcept>

#include "../Utils/TextFileParser.h"
#include "../Utils/Text.h"

Localizer::Localizer(std::string locale)
{
	auto locFile = TextFileParser::ParseFile(locale, {});

	bool locStrRead = false;
	std::string locStr;

	for (auto& line : locFile) {
		if (locStrRead) {
			_localizedStrings[locStr] = line[0];

			std::vector<uint32_t> decodedLine =
				Text::DecodeUTF8(line[0]);

			for (uint32_t c : decodedLine) {
				if (c == '\n') {
					continue;
				}

				_charSet.insert(c);
			}
		} else {
			locStr = line[0];
		}

		locStrRead = !locStrRead;
	}

	if (locStrRead) {
		throw std::runtime_error(
			"Localization file does not have last string.");
	}

	for (uint32_t c = 0x20; c <= 0x7e; ++c) {
		_charSet.insert(c);
	}
}
