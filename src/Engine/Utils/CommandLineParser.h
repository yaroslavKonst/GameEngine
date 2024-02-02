#ifndef _COMMAND_LINE_PARSER_H
#define _COMMAND_LINE_PARSER_H

#include <map>
#include <set>
#include <string>

namespace CommandLineParser
{
	struct Args
	{
		std::map<std::string, std::string> Keys;
		std::set<std::string> Args;
	};

	Args Parse(
		int argc,
		char** argv,
		std::map<std::string, std::string> defaultKeys = {});
}

#endif
