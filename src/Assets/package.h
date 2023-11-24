#ifndef _PACKAGE_H
#define _PACKAGE_H

#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include <fstream>

/*
 * Package file format.
 * 0: uint64_t. Entry number.
 * 8: Entry table:
 *    uint64_t Offset, uint64_t Size, uint64_t nameOffset, uint64_t nameSize.
*/

class Package
{
public:
	struct Mapping
	{
		uint64_t Offset;
		uint64_t Size;
	};

	Package(std::string filename);
	~Package();

	std::vector<uint8_t> GetData(std::string name);

	static void BuildPackage(
		std::string packageName,
		std::vector<std::string> entryNames,
		std::vector<std::string> fileNames);

	static Package* Instance();

	static void LoadPackage(std::string name);
	static void UnloadPackage();

private:
	std::map<std::string, Mapping> _mappings;
	std::fstream _packageFile;

	static Package* _instance;
};

#endif
