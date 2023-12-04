#ifndef _LOCALIZER_H
#define _LOCALIZER_H

#include <string>
#include <map>
#include <set>
#include <cstdint>

class Localizer
{
public:
	Localizer(std::string locale);

	std::string Localize(std::string msg)
	{
		if (_localizedStrings.find(msg) != _localizedStrings.end()) {
			return _localizedStrings[msg];
		}

		return std::string("LOC ERR: ") + msg;
	}

	const std::set<uint32_t>& GetCharSet()
	{
		return _charSet;
	}

private:
	std::map<std::string, std::string> _localizedStrings;
	std::set<uint32_t> _charSet;
};

#endif
