#include "logger.h"

#include <iostream>
#include <chrono>

namespace Logger
{
	static Level _logLevel = Level::Silent;
	static std::chrono::high_resolution_clock::time_point _programStart =
		std::chrono::high_resolution_clock::now();
	static bool _coloredLog = false;

	const char* StrLevel(Level level)
	{
		const char* silent = "Silent";
		const char* error = "Error";
		const char* errorColored = "\e[31mError\e[0m";
		const char* warning = "Warning";
		const char* warningColored = "\e[93mWarning\e[0m";
		const char* verbose = "Message";
		const char* verboseColored = "\e[94mMessage\e[0m";

		switch (level) {
		case Level::Silent:
			return silent;
		case Level::Error:
			return _coloredLog ? errorColored : error;
		case Level::Warning:
			return _coloredLog ? warningColored : warning;
		case Level::Verbose:
			return _coloredLog ? verboseColored : verbose;
		}

		return "";
	}

	std::string GetTimeString()
	{
		using namespace std::chrono;

		high_resolution_clock::time_point curr =
			high_resolution_clock::now();

		auto dur = curr - _programStart;

		uint32_t hourCount = duration_cast<hours>(dur).count();
		uint32_t minuteCount =
			duration_cast<minutes>(dur).count() % 60;
		uint32_t secondCount =
			duration_cast<seconds>(dur).count() % 60;
		uint32_t msCount =
			duration_cast<milliseconds>(dur).count() % 1000;

		std::string res;
		res += '[';
		res += std::to_string(hourCount) + ':';

		std::string minStr = std::to_string(minuteCount);

		if (minStr.size() < 2) {
			res += '0';
		}

		res += minStr + ':';

		std::string secStr = std::to_string(secondCount);

		if (secStr.size() < 2) {
			res += '0';
		}

		res += secStr + '.';

		std::string msStr = std::to_string(msCount);

		if (msStr.size() < 3) {
			for (uint32_t i = 0; i < 3 - msStr.size(); ++i) {
				res += '0';
			}
		}

		res += msStr;

		return res + ']';
	}

	Logger::Logger(Level level)
	{
		_level = level;

		if (_logLevel >= _level) {
			std::cout << GetTimeString() << " " <<
				StrLevel(_level) << ": ";
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

	Logger& Logger::operator<<(uint64_t message)
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

	Logger& Logger::operator<<(const Math::Vec<3>& message)
	{
		if (_logLevel >= _level) {
			std::cout << message[0] << ' ' <<
				message[1] << ' ' <<
				message[2];
		}

		return *this;
	}

	void SetLevel(Level level)
	{
		_logLevel = level;
	}

	void SetColored(bool value)
	{
		_coloredLog = value;
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
