#include <unistd.h>

#include "../VideoEngine/window.h"

int main(int argc, char** argv)
{
	Window window(500, 200, "Welcome");
	sleep(5);

	return 0;
}
