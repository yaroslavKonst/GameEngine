#include "demo.h"

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Verbose);

	Demo::Run();

	return 0;
}
