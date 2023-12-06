#include "compressor.h"

#include <map>
#include <set>
#include <stdexcept>
#include <cstring>

#include "PrefixTree.h"
#include "BitHelper.h"
#include "CodeTree.h"

#include "../Logger/logger.h"

#define TEST

template<typename T>
void IntToBytes(T value, uint8_t* bytes)
{
	for (uint8_t i = 0; i < sizeof(T); ++i) {
		bytes[i] = value & 0xff;
		value >>= 8;
	}
}

template<typename T>
T BytesToInt(const uint8_t* bytes)
{
	T result = 0;

	for (uint8_t i = 0; i < sizeof(T); ++i) {
		result |= bytes[i] << 8 * i;
	}

	return result;
}

static std::map<uint8_t, std::pair<size_t, std::vector<uint8_t>>>
GetSymbolCodes(const std::vector<uint8_t>& data)
{
	std::map<uint8_t, size_t> symbolCount;

	for (uint8_t s : data) {
		if (symbolCount.find(s) == symbolCount.end()) {
			symbolCount[s] = 0;
		}

		symbolCount[s] += 1;
	}

	CodeTree tree;

	for (auto& symbol : symbolCount) {
		tree.AddSymbol(symbol.first, symbol.second);
	}

	tree.Build();

	std::map<uint8_t, std::pair<size_t, std::vector<uint8_t>>> symbolCodes;

	for (auto& symbol : symbolCount) {
		size_t codeSize;
		std::vector<uint8_t> code =
			tree.GetCode(symbol.first, codeSize);

		symbolCodes[symbol.first] = {codeSize, code};
	}

	return symbolCodes;
}

std::vector<uint8_t> Compressor::Compress(const std::vector<uint8_t>& data)
{
	std::map<uint8_t, std::pair<size_t, std::vector<uint8_t>>> symbolCodes =
		GetSymbolCodes(data);

	std::vector<uint8_t> header(2);

	IntToBytes<uint16_t>(symbolCodes.size(), header.data());

	for (auto& symbolCode : symbolCodes) {
		header.push_back(symbolCode.first);
		header.push_back(symbolCode.second.first);

		for (uint8_t byte : symbolCode.second.second) {
			header.push_back(byte);
		}
	}

	BitWriter writer;

	for (uint8_t symbol : data) {
		BitReader reader(
			symbolCodes[symbol].second.data(),
			symbolCodes[symbol].first);

		uint8_t bit;
		while (reader.Get(bit)) {
			writer.Put(bit);
		}
	}

	size_t dataSize;
	std::vector<uint8_t> dataSegment = writer.Get(dataSize);

	header.resize(header.size() + 8);
	IntToBytes<uint64_t>(dataSize, header.data() + header.size() - 8);

	std::vector<uint8_t> encodedData;

	if (header.size() + dataSegment.size() < data.size()) {
		encodedData.resize(header.size() + dataSegment.size() + 1);
		encodedData[0] = 1;
		memcpy(encodedData.data() + 1, header.data(), header.size());
		memcpy(
			encodedData.data() + 1 + header.size(),
			dataSegment.data(),
			dataSegment.size());
	} else {
		encodedData.resize(data.size() + 1);
		encodedData[0] = 0;
		memcpy(encodedData.data() + 1, data.data(), data.size());
	}

#ifdef TEST
	std::vector<uint8_t> test = Decompress(encodedData);

	if (test.size() != data.size()) {
		throw std::runtime_error("Decompress test failed.");
	}

	for (size_t i = 0; i < test.size(); ++i) {
		if (test[i] != data[i]) {
			throw std::runtime_error("Decompress test failed.");
		}
	}
#endif

	return encodedData;
}

std::vector<uint8_t> Compressor::Decompress(const std::vector<uint8_t>& data)
{
	if (data[0] == 0) {
		std::vector<uint8_t> decodedData(data.size() - 1);
		memcpy(decodedData.data(), data.data() + 1, decodedData.size());
		return decodedData;
	}

	size_t dataIndex = 1;
	std::map<uint8_t, std::pair<size_t, std::vector<uint8_t>>> symbolCodes;

	size_t symbolCount = BytesToInt<uint16_t>(data.data() + dataIndex);
	dataIndex += 2;

	for (size_t symbolIdx = 0; symbolIdx < symbolCount; ++symbolIdx) {
		uint8_t symbol = data[dataIndex];
		++dataIndex;
		size_t codeSize = data[dataIndex];
		++dataIndex;

		std::vector<uint8_t> code((codeSize - 1) / 8 + 1);

		for (size_t idx = 0; idx < code.size(); ++idx) {
			code[idx] = data[dataIndex];
			++dataIndex;
		}

		symbolCodes[symbol] = {codeSize, code};
	}

	PrefixTree tree;

	for (auto& symbol : symbolCodes) {
		BitReader reader(
			symbol.second.second.data(),
			symbol.second.first);

		tree.InsertValue(symbol.first, &reader);
	}

	tree.Init();

	size_t dataSize = BytesToInt<uint64_t>(data.data() + dataIndex);
	dataIndex += 8;

	std::vector<uint8_t> decodedData;

	BitReader reader(data.data() + dataIndex, dataSize);

	uint8_t bit;
	while (reader.Get(bit)) {
		uint8_t symbol;

		bool gotSymbol = tree.NextBit(bit, symbol);

		if (gotSymbol) {
			decodedData.push_back(symbol);
		}
	}

	return decodedData;
}
