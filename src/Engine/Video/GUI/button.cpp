#include "button.h"

#include "../../Logger/logger.h"

Button::Button(Video* video, TextHandler* textHandler) :
	_label(video, textHandler)
{
	_video = video;

	_activated = false;

	_video->Subscribe(this);
}

Button::~Button()
{
	_video->Unsubscribe(this);
}

void Button::Activate()
{
	_label.Activate();

	float x = _position.x;
	float y = _position.y;

	InputArea.x0 = x - _width / 2;
	InputArea.y0 = y - _height / 2;
	InputArea.x1 = x + _width / 2;
	InputArea.y1 = y + _height / 2;
}

void Button::Deactivate()
{
	Disable();
	_label.Deactivate();
}

void Button::Enable()
{
	SetInputEnabled(true);
}

void Button::Disable()
{
	SetInputEnabled(false);

	if (_activated) {
		_label.SetImage(_image);
		_label.SetImageColor(_imageColor);
		_activated = false;
		_label.Activate();
	}
}

bool Button::MouseMove(double xpos, double ypos, bool inArea)
{
	if (!_activated && inArea) {
		_label.SetImage(_activeImage);
		_label.SetImageColor(_activeImageColor);
		_activated = true;
		_label.Activate();
	}

	if (_activated && !inArea) {
		_label.SetImage(_image);
		_label.SetImageColor(_imageColor);
		_activated = false;
		_label.Activate();
	}

	return true;
}

bool Button::MouseButton(int button, int action, int mods)
{
	_action();

	return true;
}
