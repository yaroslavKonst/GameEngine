#include "package.h"

int main(int argc, char** argv)
{
	std::vector<std::string> entryNames;

	for (int i = 2; i < argc; ++i) {
		entryNames.push_back(argv[i]);
	}

	Package::BuildPackage(argv[1], entryNames, entryNames);

	return 0;
}
