#include "../Game/world.h"
#include "../Logger/logger.h"

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Verbose);

	World world;
	world.Run();

	return 0;
}
