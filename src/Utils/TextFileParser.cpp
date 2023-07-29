#include "TextFileParser.h"

#include <fstream>
#include <stdexcept>

namespace TextFileParser
{
	File ParseFile(
		std::string path,
		std::set<char> wordDelims,
		std::set<char> lineDelims)
	{
		std::fstream inFile(path, std::ios::in);

		if (!inFile.is_open()) {
			throw std::runtime_error(
				"Failed to open file for parsing.");
		}

		File file;
		Line line;
		std::string word;

		char currChar;

		while (!inFile.get(currChar).eof()) {
			bool isWordDelim =
				wordDelims.find(currChar) != wordDelims.end();
			bool isLineDelim =
				lineDelims.find(currChar) != lineDelims.end();

			if (isWordDelim || isLineDelim) {
				if (!word.empty()) {
					line.push_back(word);
				}

				word.clear();

				if (isLineDelim) {
					if (!line.empty()) {
						file.push_back(line);
					}

					line.clear();
				}
			} else {
				word.push_back(currChar);
			}
		}

		if (!word.empty()) {
			line.push_back(word);
			file.push_back(line);
		}

		return file;
	}
}
