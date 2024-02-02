#include "../Engine/Video/video.h"

int main(int argc, char** argv)
{
	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 1;

	Video* video = new Video(
		1400,
		1000,
		"Soft Body Tool",
		"Soft Body Tool",
		&videoSettings);

	video->SetFOV(80);
	video->SetCameraUp({0, 0, 1});

	video->MainLoop();

	delete video;

	return 0;
}
