#ifndef _TEXTBOX_H
#define _TEXTBOX_H

#include "video.h"
#include "TextHandler.h"

class TextBox
{
public:
	TextBox(Video* video, TextHandler* textHandler);
	~TextBox();

	void SetText(std::string text);
	void SetPosition(float x, float y);
	void SetTextSize(float size);
	void SetTextColor(const glm::vec4& value)
	{
		_color = value;
	}

	void SetDepth(float value)
	{
		_depth = value;
	}

	void Place();

	void Activate();
	void Deactivate();

	float GetWidth()
	{
		return _width;
	}

private:
	Video* _video;
	TextHandler* _textHandler;
	std::string _text;
	std::vector<Rectangle*> _line;
	glm::vec2 _position;
	glm::vec4 _color;

	bool _textUpdated;
	float _textSize;
	float _width;
	float _depth;
};

#endif
