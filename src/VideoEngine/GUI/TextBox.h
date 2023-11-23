#ifndef _TEXTBOX_H
#define _TEXTBOX_H

#include "../video.h"
#include "../TextHandler.h"

class TextBox
{
public:
	enum class Alignment
	{
		Left,
		Center,
		Right
	};

	TextBox(Video* video, TextHandler* textHandler);
	~TextBox();

	void SetText(std::string text);
	void SetPosition(
		float x,
		float y,
		Alignment alignment = Alignment::Left);
	void SetTextSize(float size);
	void SetTextColor(const glm::vec4& value)
	{
		_color = value;
		_positionUpdated = false;
	}

	void SetDepth(float value)
	{
		_depth = value;
		_positionUpdated = false;
	}

	void Activate();
	void Deactivate();

	float GetWidth()
	{
		return _width;
	}

private:
	Video* _video;
	TextHandler* _textHandler;
	std::vector<uint32_t> _text;
	std::vector<Rectangle*> _line;
	glm::vec2 _position;
	Alignment _alignment;
	glm::vec4 _color;

	bool _textUpdated;
	bool _positionUpdated;
	float _textSize;
	float _width;
	float _depth;

	void Place();
};

#endif
