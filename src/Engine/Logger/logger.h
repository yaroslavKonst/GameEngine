#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>
#include <cstdint>

#include "../Math/vec.h"

namespace Logger
{
	enum class Level
	{
		Silent = 0,
		Error = 1,
		Warning = 2,
		Verbose = 3
	};

	class Logger
	{
	public:
		Logger(Level level);
		~Logger();

		Logger& operator<<(const std::string& message);
		Logger& operator<<(const char* message);
		Logger& operator<<(int32_t message);
		Logger& operator<<(uint32_t message);
		Logger& operator<<(uint64_t message);
		Logger& operator<<(float message);
		Logger& operator<<(double message);
		Logger& operator<<(const Math::Vec<3>& message);

	private:
		Level _level;
	};

	void SetLevel(Level level);
	void SetColored(bool value);

	Logger Error();
	Logger Warning();
	Logger Verbose();
}

#endif
