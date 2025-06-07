#include "program.h"

#include "../../Engine/Utils/loader.h"
#include "../../Engine/Utils/Text.h"
#include "../../Engine/Logger/logger.h"
#include "menu.h"

static void VideoThread(Video* video)
{
	video->MainLoop();
}

Program::Program()
{
	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 1;
	videoSettings.Scaling = 1.5;

	_engine.video = new Video(1400, 1000, "Game", "Game", &videoSettings);
	_engine.audio = new Audio;
	_engine.universe = new TimeEngine(20, _engine.video);

	_engine.video->SetFOV(80);
	_engine.video->SetCameraUp({0, 0, 1});
	_engine.video->SetCameraTarget({1, 0, 0});

	_engine.localizer = new Localizer("Locale/en");

	auto glyphs = Text::LoadFont(
		"Fonts/DroidSans.ttf",
		_engine.localizer->GetCharSet());

	_engine.textHandler = new TextHandler(_engine.video, glyphs);
}

Program::~Program()
{
	delete _engine.textHandler;
	delete _engine.localizer;
	delete _engine.universe;
	delete _engine.audio;
	delete _engine.video;
}

void Program::Run()
{
	Menu menu(&_engine);

	std::thread* videoThread =
		new std::thread(VideoThread, _engine.video);

	_engine.universe->MainLoop();

	_engine.video->Stop();
	videoThread->join();
	delete videoThread;
}
