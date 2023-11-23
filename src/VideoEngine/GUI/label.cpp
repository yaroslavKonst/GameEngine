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

void Label::SetText(std::string text)
{
	_textBox.SetText(text);
}

void Label::SetPosition(float x, float y, TextBox::Alignment alignment)
{
	_position = {x, y};
	_alignment = alignment;
}

void Label::SetSize(float width, float height)
{
	_width = width;
	_height = height;
}

void Label::SetTextSize(float size)
{
	_textBox.SetTextSize(size);
	_textSize = size;
}

void Label::SetTextColor(const glm::vec4& value)
{
	_textBox.SetTextColor(value);
}

void Label::SetImage(uint32_t texture)
{
	_image.SetTexture({texture});
}

void Label::SetImageColor(const glm::vec4& value)
{
	_image.SetColorMultiplier(value);
}

void Label::SetDepth(float value)
{
	_image.SetRectangleDepth(value);
	_textBox.SetDepth(value + 0.01);
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
	_image.SetRectanglePosition({
		x - _width / 2,
		y - _height / 2,
		x + _width / 2,
		y + _height / 2
	});

	_image.SetDrawEnabled(true);
	_textBox.Activate();
}

void Label::Deactivate()
{
	_image.SetDrawEnabled(false);
	_textBox.Deactivate();
}
