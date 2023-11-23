#ifndef _LABEL_H
#define _LABEL_H

#include "TextBox.h"

class Label
{
public:
	Label(Video* video, TextHandler* textHandler);
	~Label();

	void SetText(std::string text);
	void SetPosition(
		float x,
		float y,
		TextBox::Alignment alignment = TextBox::Alignment::Center);

	void SetSize(float width, float height);

	void SetTextSize(float size);
	void SetTextColor(const glm::vec4& value);

	void SetImage(uint32_t texture);
	void SetImageColor(const glm::vec4& value);

	void SetDepth(float value);

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
