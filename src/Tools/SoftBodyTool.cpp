#include "../Engine/Video/video.h"
#include "../Engine/Video/GUI/button.h"
#include "../Engine/Logger/logger.h"
#include "../Engine/Assets/localizer.h"
#include "../Engine/Assets/package.h"

class App : public InputHandler
{
public:
	App(Video* video)
	{
		_video = video;

		_work = true;

		_video->SetFOV(80);
		_video->SetCameraUp({0, 0, 1});

		_localizer = new Localizer("Locale/en");

		auto glyphs = Text::LoadFont(
			"Fonts/DroidSans.ttf",
			_localizer->GetCharSet());

		_textHandler = new TextHandler(_video, glyphs);
	}

	~App()
	{
		delete _textHandler;
		delete _localizer;
	}

	void MainLoop()
	{
		Logger::Verbose() << "App started.";

		while (_work) {
			usleep(10000);
		}

		Logger::Verbose() << "App stopped.";
	}

	void Stop()
	{
		_work = false;
	}

	bool InInputArea(float x, float y) override
	{
		return true;
	}

private:
	std::atomic<bool> _work;

	Video* _video;
	Localizer* _localizer;
	TextHandler* _textHandler;
};

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Verbose);
	Package::LoadPackage("resources.bin");

	Video::GraphicsSettings videoSettings{};
	videoSettings.MsaaLimit = 1;

	Video* video = new Video(
		1400,
		1000,
		"Soft Body Tool",
		"Soft Body Tool",
		&videoSettings);

	App* app = new App(video);

	std::thread* appThread = new std::thread([app]() {app->MainLoop();});

	video->MainLoop();

	app->Stop();
	appThread->join();
	delete appThread;

	delete app;
	delete video;

	Package::UnloadPackage();

	return 0;
}
