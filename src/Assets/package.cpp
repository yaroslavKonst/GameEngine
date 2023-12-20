#include "package.h"

#include <iostream>

#include "../Utils/compressor.h"

Package* Package::_instance = nullptr;

Package* Package::Instance()
{
	if (!_instance) {
		throw std::runtime_error("Package file is not loaded.");
	}

	return _instance;
}

void Package::LoadPackage(std::string name)
{
	if (_instance) {
		UnloadPackage();
	}

	_instance = new Package(name);
}

void Package::UnloadPackage()
{
	if (_instance) {
		delete _instance;
		_instance = nullptr;
	}
}

Package::Package(std::string filename)
{
	_packageFile.open(filename, std::ios::in | std::ios::binary);

	if (!_packageFile.is_open()) {
		throw std::runtime_error("Failed to open package file " +
			filename);
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
		std::fstream file;
		file.open(
			name,
			std::ios::in | std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error(
				std::string("Package does not contain entry ") +
				name);
		}

		std::vector<uint8_t> data(file.tellg());

		file.seekg(0);
		file.read((char*)data.data(), data.size());
		file.close();

		return data;
	}

	Mapping mapping = _mappings[name];

	_packageFile.seekg(mapping.Offset);

	std::vector<uint8_t> result(mapping.Size);
	_packageFile.read((char*)result.data(), result.size());

	return Compressor::Decompress(result);
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

		std::vector<uint8_t> compressedEntryData =
			Compressor::Compress(entryData);

		std::cout << fileNames[entryIndex] << ": ";

		if (compressedEntryData[0] == 0) {
			std::cout << "stored";
		} else {
			std::cout << "compressed, layers: " <<
				(int)compressedEntryData[0] << " (" <<
				(float)compressedEntryData.size() /
				entryData.size() << ")";
		}

		std::cout << std::endl;

		tableEntry[0] = packageFile.tellp();
		tableEntry[1] = compressedEntryData.size();

		packageFile.write(
			(char*)compressedEntryData.data(),
			compressedEntryData.size());

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
