#include <unistd.h>

#include "../VideoEngine/video.h"

int main(int argc, char** argv)
{
	Video window(500, 200, "Welcome", "Application");
	sleep(5);

	return 0;
}
