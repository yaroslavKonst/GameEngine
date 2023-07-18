#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>

std::vector<uint8_t> ReadFile(std::string path)
{
	std::fstream file;
	file.open(path, std::ios::in | std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file.");
	}

	size_t size = file.tellg();
	std::vector<uint8_t> buffer(size);

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), size);
	file.close();

	return buffer;
}

uint8_t GetChar(uint8_t value)
{
	if (value < 10) {
		return value + '0';
	}

	return value - 10 + 'a';
}

std::string Convert(uint8_t value)
{
	uint8_t d0 = value & 0x0f;
	uint8_t d1 = (value & 0xf0) >> 4;

	std::string res;

	res.push_back(GetChar(d1));
	res.push_back(GetChar(d0));

	return res;
}

int main(int argc, char** argv)
{
	std::string shaderName = argv[1];
	std::string inPath = argv[2];
	std::string outPath = argv[3];

	std::vector<uint8_t> data = ReadFile(inPath);

	std::fstream outFile;
	outFile.open(outPath, std::ios::out);

	outFile << "const uint8_t " << shaderName << "[] = {" << std::endl;

	for (size_t i = 0; i < data.size(); ++i) {
		if (i > 0) {
			outFile << ", ";
		}

		outFile << "0x" << Convert(data[i]);
	}

	outFile << std::endl << "};" << std::endl;

	outFile.close();
}
