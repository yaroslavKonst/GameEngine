#include "package.h"

Package* Package::_instance = nullptr;

Package* Package::Instance()
{
	if (!_instance) {
		_instance = new Package("resources.bin");
	}

	return _instance;
}

Package::Package(std::string filename)
{
	_packageFile.open(filename, std::ios::in | std::ios::binary);

	if (!_packageFile.is_open()) {
		throw std::runtime_error("Failed to open package file.");
	}

	uint64_t entryNumber;
	_packageFile.read((char*)&entryNumber, sizeof(uint64_t));

	std::vector<std::vector<uint64_t>> entryData(entryNumber);

	for (uint64_t entryIndex = 0; entryIndex < entryNumber; ++entryIndex) {
		std::vector<uint64_t> entry(4);

		_packageFile.read(
			(char*)entry.data(),
			sizeof(uint64_t) * entry.size());

		entryData[entryIndex] = entry;
	}

	for (uint64_t entryIndex = 0; entryIndex < entryNumber; ++entryIndex) {
		std::vector<int8_t> entryNameData(entryData[entryIndex][3]);

		_packageFile.seekg(entryData[entryIndex][2]);
		_packageFile.read(
			(char*)entryNameData.data(),
			entryNameData.size());

		std::string entryName;

		for (size_t i = 0; i < entryNameData.size(); ++i) {
			entryName += entryNameData[i];
		}

		_mappings[entryName] = Mapping({
			entryData[entryIndex][0],
			entryData[entryIndex][1]
		});
	}
}

Package::~Package()
{
	_packageFile.close();
}

std::vector<uint8_t> Package::GetData(std::string name)
{
	if (_mappings.find(name) == _mappings.end()) {
		throw std::runtime_error(
			std::string("Package does not contain entry ") + name);
	}

	Mapping mapping = _mappings[name];

	_packageFile.seekg(mapping.Offset);

	std::vector<uint8_t> result(mapping.Size);
	_packageFile.read((char*)result.data(), result.size());

	return result;
}

void Package::BuildPackage(
	std::string packageName,
	std::vector<std::string> entryNames,
	std::vector<std::string> fileNames)
{
	uint64_t entryNumber = entryNames.size();

	std::vector<std::vector<uint64_t>> entryTable(entryNumber);

	std::fstream packageFile;
	packageFile.open(packageName, std::ios::out | std::ios::binary);

	packageFile.write((char*)&entryNumber, sizeof(uint64_t));

	packageFile.seekp(sizeof(uint64_t) * (entryNumber * 4 + 1));

	for (uint64_t entryIndex = 0; entryIndex < entryNumber; ++entryIndex) {
		std::vector<uint64_t> tableEntry(4);

		tableEntry[2] = packageFile.tellp();
		tableEntry[3] = entryNames[entryIndex].size();

		packageFile.write(
			entryNames[entryIndex].c_str(),
			tableEntry[3]);

		std::fstream entryFile;
		entryFile.open(
			fileNames[entryIndex],
			std::ios::in | std::ios::ate | std::ios::binary);

		if (!entryFile.is_open()) {
			throw std::runtime_error(
				std::string("Failed to open file ") +
				fileNames[entryIndex]);
		}

		size_t entrySize = entryFile.tellg();
		std::vector<uint8_t> entryData(entrySize);

		entryFile.seekg(0);
		entryFile.read((char*)entryData.data(), entrySize);
		entryFile.close();

		tableEntry[0] = packageFile.tellp();
		tableEntry[1] = entrySize;

		packageFile.write((char*)entryData.data(), entrySize);

		entryTable[entryIndex] = tableEntry;
	}

	packageFile.seekp(sizeof(uint64_t));

	for (uint64_t entryIndex = 0; entryIndex < entryNumber; ++entryIndex) {
		packageFile.write(
			(char*)entryTable[entryIndex].data(),
			sizeof(uint64_t) * 4);
	}

	packageFile.close();
}
