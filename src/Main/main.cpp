#include <unistd.h>

#include "../VideoEngine/video.h"
#include "../Logger/logger.h"

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Verbose);

	Video window(500, 200, "Welcome", "Application");
	sleep(5);

	return 0;
}
