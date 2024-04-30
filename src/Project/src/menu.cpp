#include "menu.h"

#include "../../Engine/Logger/logger.h"

Menu::Menu(Engine* engine)
{
	_engine = engine;

	LoadResources();

	_world = new World(
		_engine,
		[this]() -> void
		{
			_world->Unload();
			ShowMainMenu();
			_inGame = false;
			_inPause = false;
		});

	LoadButtons();

	SetInputLayer(10);
	_engine->video->Subscribe(this);
	SetInputEnabled(true);

	_inGame = false;
	_inPause = false;
	ShowMainMenu();
}

Menu::~Menu()
{
	SetInputEnabled(false);
	_engine->video->Unsubscribe(this);

	UnloadButtons();

	delete _world;

	UnloadResources();
}

void Menu::LoadResources()
{
	auto buttonBackgroundImage = Loader::LoadImage(
		"Menu/ButtonBackground.png");
	_buttonBackground = _engine->video->LoadTexture(buttonBackgroundImage);
}

void Menu::UnloadResources()
{
	_engine->video->UnloadTexture(_buttonBackground);
}

void Menu::LoadButtons()
{
	_playButton = new Button(_engine->video, _engine->textHandler);
	_exitButton = new Button(_engine->video, _engine->textHandler);
	_continueButton = new Button(_engine->video, _engine->textHandler);
	_exitToMenuButton = new Button(_engine->video, _engine->textHandler);

	_playButton->SetText(_engine->localizer->Localize("Play"));
	_exitButton->SetText(_engine->localizer->Localize("Exit"));
	_continueButton->SetText(_engine->localizer->Localize("Continue"));
	_exitToMenuButton->SetText(_engine->localizer->Localize("Exit"));

	_playButton->SetPosition(0, -0.2);
	_exitButton->SetPosition(0, 0.1);
	_continueButton->SetPosition(-0.6, -0.2);
	_exitToMenuButton->SetPosition(-0.6, 0.1);

	_playButton->SetSize(0.5, 0.1);
	_exitButton->SetSize(0.5, 0.1);
	_continueButton->SetSize(0.5, 0.1);
	_exitToMenuButton->SetSize(0.5, 0.1);

	_playButton->SetTextSize(0.05);
	_exitButton->SetTextSize(0.05);
	_continueButton->SetTextSize(0.05);
	_exitToMenuButton->SetTextSize(0.05);

	_playButton->SetTextColor({1, 1, 1, 1});
	_exitButton->SetTextColor({1, 1, 1, 1});
	_continueButton->SetTextColor({1, 1, 1, 1});
	_exitToMenuButton->SetTextColor({1, 1, 1, 1});

	_playButton->SetImage(_buttonBackground);
	_exitButton->SetImage(_buttonBackground);
	_continueButton->SetImage(_buttonBackground);
	_exitToMenuButton->SetImage(_buttonBackground);
	_playButton->SetActiveImage(_buttonBackground);
	_exitButton->SetActiveImage(_buttonBackground);
	_continueButton->SetActiveImage(_buttonBackground);
	_exitToMenuButton->SetActiveImage(_buttonBackground);

	_playButton->SetImageColor({1, 1, 1, 1});
	_exitButton->SetImageColor({1, 1, 1, 1});
	_continueButton->SetImageColor({1, 1, 1, 1});
	_exitToMenuButton->SetImageColor({1, 1, 1, 1});
	_playButton->SetActiveImageColor({0.6, 0.6, 0.6, 1});
	_exitButton->SetActiveImageColor({0.6, 0.6, 0.6, 1});
	_continueButton->SetActiveImageColor({0.6, 0.6, 0.6, 1});
	_exitToMenuButton->SetActiveImageColor({0.6, 0.6, 0.6, 1});

	_playButton->SetDepth(1);
	_exitButton->SetDepth(2);
	_continueButton->SetDepth(3);
	_exitToMenuButton->SetDepth(4);

	_playButton->SetAction([this]() -> void {PlayButtonPressed();});
	_exitButton->SetAction([this]() -> void {ExitButtonPressed();});
	_continueButton->SetAction(
		[this]() -> void {ContinueButtonPressed();});
	_exitToMenuButton->SetAction(
		[this]() -> void {ExitToMenuButtonPressed();});
}

void Menu::UnloadButtons()
{
	_playButton->Deactivate();
	_exitButton->Deactivate();
	_continueButton->Deactivate();
	_exitToMenuButton->Deactivate();
	delete _playButton;
	delete _exitButton;
	delete _continueButton;
	delete _exitToMenuButton;
}

void Menu::ShowMainMenu()
{
	_playButton->Activate();
	_exitButton->Activate();
	_playButton->Enable();
	_exitButton->Enable();
}

void Menu::HideMainMenu()
{
	_playButton->Deactivate();
	_exitButton->Deactivate();
}

void Menu::ShowPauseMenu()
{
	_continueButton->Activate();
	_exitToMenuButton->Activate();
	_continueButton->Enable();
	_exitToMenuButton->Enable();
}

void Menu::HidePauseMenu()
{
	_continueButton->Deactivate();
	_exitToMenuButton->Deactivate();
}

void Menu::PlayButtonPressed()
{
	HideMainMenu();
	_world->Load();
	_inGame = true;
	_inPause = false;

	_engine->video->ToggleRawMouseInput();
}

void Menu::ExitButtonPressed()
{
	HideMainMenu();
	_engine->universe->Stop();
}

void Menu::ContinueButtonPressed()
{
	HidePauseMenu();
	_world->Continue();
	_inGame = true;
	_inPause = false;
}

void Menu::ExitToMenuButtonPressed()
{
	_world->Unload();
	HidePauseMenu();
	ShowMainMenu();
	_inGame = false;
	_inPause = false;
}

void Menu::Key(int key, int scancode, int action, int mods)
{
	bool escPressed = key == GLFW_KEY_ESCAPE && action == GLFW_PRESS;

	if (escPressed) {
		ProcessEscape();
	}
}

bool Menu::WindowClose()
{
	if (_inGame || _inPause) {
		_world->Unload();
	}

	ExitButtonPressed();

	return true;
}

void Menu::ProcessEscape()
{
	if (_inGame) {
		_world->Pause();
		ShowPauseMenu();
		_inGame = false;
		_inPause = true;
	} else if (_inPause) {
		ContinueButtonPressed();
	}
}
