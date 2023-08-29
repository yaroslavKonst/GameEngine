#include "TextFileParser.h"

#include <stdexcept>

#include "../Assets/package.h"

namespace TextFileParser
{
	File ParseFile(
		std::string name,
		std::set<char> wordDelims,
		std::set<char> lineDelims)
	{
		auto inFile = Package::Instance()->GetData(name);
		size_t currPos = 0;

		File file;
		Line line;
		std::string word;

		char currChar;

		while (currPos < inFile.size()) {
			currChar = inFile[currPos];
			++currPos;

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
