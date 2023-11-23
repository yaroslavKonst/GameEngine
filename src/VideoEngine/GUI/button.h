#ifndef _BUTTON_H
#define _BUTTON_H

#include <functional>

#include "label.h"
#include "../InputControl.h"

class Button : private InputHandler
{
public:
	Button(Video* video, TextHandler* textHandler);
	~Button();

	void SetText(std::string text)
	{
		_label.SetText(text);
	}

	void SetPosition(
		float x,
		float y,
		TextBox::Alignment alignment = TextBox::Alignment::Center)
	{
		_position = {x, y};
		_label.SetPosition(x, y, alignment);
	}

	void SetSize(float width, float height)
	{
		_width = width;
		_height = height;

		_label.SetSize(width, height);
	}

	void SetTextSize(float size)
	{
		_label.SetTextSize(size);
	}

	void SetTextColor(const glm::vec4& value)
	{
		_label.SetTextColor(value);
	}

	void SetImage(uint32_t texture)
	{
		_label.SetImage(texture);
		_image = texture;
	}

	void SetImageColor(const glm::vec4& value)
	{
		_label.SetImageColor(value);
		_imageColor = value;
	}

	void SetActiveImage(uint32_t texture)
	{
		_activeImage = texture;
	}

	void SetActiveImageColor(const glm::vec4& value)
	{
		_activeImageColor = value;
	}

	void SetDepth(float value)
	{
		_label.SetDepth(value);
		SetInputLayer(-value);
	}

	void SetAction(std::function<void()> action)
	{
		_action = action;
	}

	void Activate();
	void Deactivate();

	void Enable();
	void Disable();

private:
	Video* _video;

	Label _label;
	uint32_t _image;
	glm::vec4 _imageColor;
	uint32_t _activeImage;
	glm::vec4 _activeImageColor;

	glm::vec2 _position;
	float _width;
	float _height;

	bool _activated;

	std::function<void()> _action;

	bool MouseMove(double xpos, double ypos, bool inArea) override;
	bool MouseButton(int button, int action, int mods) override;
};

#endif
