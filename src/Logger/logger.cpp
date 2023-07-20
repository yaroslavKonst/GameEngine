#include "logger.h"

#include <iostream>

namespace Logger
{
	static Level _logLevel = Level::Silent;

	const char* StrLevel(Level level)
	{
		switch (level) {
		case Level::Silent:
			return "Silent";
		case Level::Error:
			return "Error";
		case Level::Warning:
			return "Warning";
		case Level::Verbose:
			return "Verbose";
		}

		return "";
	}

	Logger::Logger(Level level)
	{
		_level = level;

		if (_logLevel >= _level) {
			std::cout << StrLevel(_level) << ": ";
		}
	}

	Logger::~Logger()
	{
		if (_logLevel >= _level) {
			std::cout << std::endl;
		}
	}

	Logger& Logger::operator<<(const std::string& message)
	{
		if (_logLevel >= _level) {
			std::cout << message;
		}

		return *this;
	}

	Logger& Logger::operator<<(const char* message)
	{
		if (_logLevel >= _level) {
			std::cout << message;
		}

		return *this;
	}

	Logger& Logger::operator<<(int32_t message)
	{
		if (_logLevel >= _level) {
			std::cout << message;
		}

		return *this;
	}

	Logger& Logger::operator<<(uint32_t message)
	{
		if (_logLevel >= _level) {
			std::cout << message;
		}

		return *this;
	}

	Logger& Logger::operator<<(float message)
	{
		if (_logLevel >= _level) {
			std::cout << message;
		}

		return *this;
	}

	Logger& Logger::operator<<(double message)
	{
		if (_logLevel >= _level) {
			std::cout << message;
		}

		return *this;
	}

	void SetLevel(Level level)
	{
		_logLevel = level;
	}

	Logger Error()
	{
		return Logger(Level::Error);
	}

	Logger Warning()
	{
		return Logger(Level::Warning);
	}

	Logger Verbose()
	{
		return Logger(Level::Verbose);
	}
}
