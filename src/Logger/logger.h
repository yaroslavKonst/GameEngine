#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>

namespace Logger
{
	enum class Level
	{
		Silent = 0,
		Error = 1,
		Warning = 2,
		Verbose = 3
	};

	void SetLevel(Level level);

	void Error(std::string message);
	void Warning(std::string message);
	void Verbose(std::string message);
}

#endif
