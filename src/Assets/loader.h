#ifndef _LOADER_H
#define _LOADER_H

#include <string>
#include <stdexcept>
#include <vector>
#include <cstdint>

namespace Loader
{
	std::vector<uint8_t> LoadImage(
		std::string file,
		int& width,
		int& height);
}

#endif
