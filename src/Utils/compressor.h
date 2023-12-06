#ifndef _COMPRESSOR_H
#define _COMPRESSOR_H

#include <cstdint>
#include <vector>

namespace Compressor
{
	// Format:
	// 1 byte: 1: compressed data. 0: stored data.
	// 2 bytes: number of unique symbols.
	// N symbol descriptors:
	//   1 byte: symbol.
	//   1 byte: code length in bits.
	//   M bytes: code (packed bits).
	// 8 bytes: data length in bits.
	// Data block.
	std::vector<uint8_t> Compress(const std::vector<uint8_t>& data);
	std::vector<uint8_t> Decompress(const std::vector<uint8_t>& data);
}

#endif
