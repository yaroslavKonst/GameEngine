#include "program.h"

#include "../Engine/Utils/loader.h"
#include "../Engine/Utils/Text.h"
#include "../Engine/Logger/logger.h"
#include "menu.h"

static void VideoThread(Video* video)
{
	video->MainLoop();
}

Program::Program()
{
	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 2;

	_common.video = new Video(1400, 1000, "Game", "Game", &videoSettings);
	_common.audio = new Audio;
	_common.universe = new TimeEngine(10, _common.video);

	_common.video->SetFOV(80);
	_common.video->SetCameraUp({0, 0, 1});
	_common.video->SetCameraTarget({1, 0, 0});

	_common.localizer = new Localizer("Locale/en");

	auto glyphs = Text::LoadFont(
		"Fonts/DroidSans.ttf",
		_common.localizer->GetCharSet());

	_common.textHandler = new TextHandler(_common.video, glyphs);
}

Program::~Program()
{
	delete _common.textHandler;
	delete _common.localizer;
	delete _common.universe;
	delete _common.audio;
	delete _common.video;
}

void Program::Run()
{
	Menu menu(&_common);

	std::thread* videoThread =
		new std::thread(VideoThread, _common.video);

	_common.universe->MainLoop();

	_common.video->Stop();
	videoThread->join();
	delete videoThread;
}
