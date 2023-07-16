#include <unistd.h>

#include "../VideoEngine/window.h"
#include "../VideoEngine/VkInstanceHandler.h"

int main(int argc, char** argv)
{
	Window window(500, 200, "Welcome");
	VkInstanceHandler::IncRef();
	sleep(5);
	VkInstanceHandler::DecRef();

	return 0;
}
