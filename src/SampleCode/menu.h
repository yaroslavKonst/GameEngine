#ifndef _MENU_H
#define _MENU_H

#include "../Engine/Video/GUI/button.h"
#include "global.h"
#include "world.h"

class Menu : public InputHandler
{
public:
	Menu(Common* common);
	~Menu();

	bool InInputArea(float x, float y) override
	{
		return true;
	}

	void Key(int key, int scancode, int action, int mods) override;
	void ProcessEscape();
	bool WindowClose() override;

private:
	Common* _common;

	uint32_t _buttonBackground;

	void LoadResources();
	void UnloadResources();

	void LoadButtons();
	void UnloadButtons();

	// Main menu
	Button* _playButton;
	Button* _exitButton;
	void ShowMainMenu();
	void HideMainMenu();
	void PlayButtonPressed();
	void ExitButtonPressed();

	// Pause menu
	bool _inGame;
	bool _inPause;
	Button* _continueButton;
	Button* _exitToMenuButton;
	void ShowPauseMenu();
	void HidePauseMenu();
	void ContinueButtonPressed();
	void ExitToMenuButtonPressed();

	World* _world;
};

#endif
