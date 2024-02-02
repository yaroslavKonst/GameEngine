#include "label.h"

#include "../../Logger/logger.h"

Label::Label(Video* video, TextHandler* textHandler) :
	_textBox(video, textHandler)
{
	_video = video;

	_video->RegisterRectangle(&_image);
}

Label::~Label()
{
	_video->RemoveRectangle(&_image);
}

void Label::Activate()
{
	float x = _position.x;
	float y = _position.y;

	float tbX = x;
	float tbY = y + _textSize / 2.0;

	switch (_alignment) {
	case TextBox::Alignment::Left:
		tbX = x - _width / 2;
		break;
	case TextBox::Alignment::Center:
		tbX = x;
		break;
	case TextBox::Alignment::Right:
		tbX = x + _width / 2;
		break;
	}

	_textBox.SetPosition(tbX, tbY, _alignment);
	_image.RectangleParams.Position = {
		x - _width / 2,
		y - _height / 2,
		x + _width / 2,
		y + _height / 2
	};

	_image.DrawParams.Enabled = true;
	_textBox.Activate();
}

void Label::Deactivate()
{
	_image.DrawParams.Enabled = false;
	_textBox.Deactivate();
}
