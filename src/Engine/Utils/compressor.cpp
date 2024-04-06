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

static std::vector<uint8_t> Decode(const std::vector<uint8_t>& data);

static std::vector<uint8_t> Encode(const std::vector<uint8_t>& data)
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

	encodedData.resize(header.size() + dataSegment.size());
	memcpy(encodedData.data(), header.data(), header.size());
	memcpy(
		encodedData.data() + header.size(),
		dataSegment.data(),
		dataSegment.size());

#ifdef TEST
	std::vector<uint8_t> test = Decode(encodedData);

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

static std::vector<uint8_t> Decode(const std::vector<uint8_t>& data)
{
	size_t dataIndex = 0;
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
	decodedData.reserve(dataSize / 8 + 1);

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

std::vector<uint8_t> Compressor::Compress(const std::vector<uint8_t>& data)
{
	uint8_t layers = 0;

	std::vector<uint8_t>* dataSegment = new std::vector<uint8_t>();
	std::vector<uint8_t>* nextDataSegment = new std::vector<uint8_t>();

	*dataSegment = data;

	bool limitReached = false;

	do {
		*nextDataSegment = Encode(*dataSegment);

		if (nextDataSegment->size() < dataSegment->size()) {
			std::vector<uint8_t>* tmp = nextDataSegment;
			nextDataSegment = dataSegment;
			dataSegment = tmp;
			++layers;
		} else {
			limitReached = true;
		}
	} while (!limitReached);

	delete nextDataSegment;

	std::vector<uint8_t> result(dataSegment->size() + 1);
	result[0] = layers;
	memcpy(result.data() + 1, dataSegment->data(), dataSegment->size());

	delete dataSegment;

	return result;
}

std::vector<uint8_t> Compressor::Decompress(const std::vector<uint8_t>& data)
{
	uint8_t layers = data[0];

	std::vector<uint8_t> dataSegment(data.size() - 1);
	memcpy(dataSegment.data(), data.data() + 1, dataSegment.size());

	while (layers) {
		--layers;
		dataSegment = Decode(dataSegment);
	}

	return dataSegment;
}
