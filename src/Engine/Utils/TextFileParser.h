#ifndef _TEXT_FILE_PARSER_H
#define _TEXT_FILE_PARSER_H

#include <vector>
#include <set>
#include <string>

namespace TextFileParser
{
	typedef std::vector<std::string> Line;
	typedef std::vector<Line> File;

	File ParseFile(
		std::string name,
		std::set<char> wordDelims = {' '},
		std::set<char> lineDelims = {'\n'});
}

#endif
