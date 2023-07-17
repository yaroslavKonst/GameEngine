#include "logger.h"

#include <iostream>

namespace Logger
{
	static Level _logLevel = Level::Silent;

	void SetLevel(Level level)
	{
		_logLevel = level;
	}

	void Error(std::string message)
	{
		if (_logLevel >= Level::Error)
		{
			std::cout << "Error: " << message << std::endl;
		}
	}

	void Warning(std::string message)
	{
		if (_logLevel >= Level::Warning)
		{
			std::cout << "Warning: " << message << std::endl;
		}
	}

	void Verbose(std::string message)
	{
		if (_logLevel >= Level::Verbose)
		{
			std::cout << "Verbose: " << message << std::endl;
		}
	}
}
