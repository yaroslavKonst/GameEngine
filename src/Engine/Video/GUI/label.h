#ifndef _LABEL_H
#define _LABEL_H

#include "TextBox.h"

class Label
{
public:
	Label(Video* video, TextHandler* textHandler);
	~Label();

	void SetText(std::string text)
	{
		_textBox.SetText(text);
	}

	void SetPosition(
		float x,
		float y,
		TextBox::Alignment alignment = TextBox::Alignment::Center)
	{
		_position = {x, y};
		_alignment = alignment;
	}

	void SetSize(float width, float height)
	{
		_width = width;
		_height = height;
	}

	void SetTextSize(float size)
	{
		_textBox.SetTextSize(size);
		_textSize = size;
	}

	void SetTextColor(const glm::vec4& value)
	{
		_textBox.SetTextColor(value);
	}

	void SetImage(uint32_t texture)
	{
		_image.RectangleParams.Texture = texture;
	}

	void SetImageColor(const glm::vec4& value)
	{
		_image.DrawParams.ColorMultiplier = value;
	}

	void SetDepth(float value)
	{
		_image.RectangleParams.Depth = value;
		_textBox.SetDepth(value + 0.01);
	}

	void Activate();
	void Deactivate();

private:
	Video* _video;

	TextBox _textBox;
	TextBox::Alignment _alignment;

	Rectangle _image;
	glm::vec2 _position;
	float _width;
	float _height;

	float _textSize;
};

#endif
