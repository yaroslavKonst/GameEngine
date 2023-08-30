#include "TextBox.h"

#include "../Logger/logger.h"

TextBox::TextBox(Video* video, TextHandler* textHandler)
{
	_video = video;
	_textHandler = textHandler;
	_textUpdated = false;
	_positionUpdated = false;
	_color = glm::vec4(1.0f);
}

TextBox::~TextBox()
{
	for (auto rectangle : _line) {
		if (rectangle) {
			_video->RemoveRectangle(rectangle);
			delete rectangle;
		}
	}
}

void TextBox::SetText(std::string text)
{
	_text = text;
	_textUpdated = false;
}

void TextBox::SetPosition(float x, float y)
{
	_position = {x, y};
	_positionUpdated = false;
}

void TextBox::SetTextSize(float size)
{
	_textSize = size;
}

void TextBox::Place()
{
	if (!_textUpdated) {
		for (auto rectangle : _line) {
			if (rectangle) {
				_video->RemoveRectangle(rectangle);
				delete rectangle;
			}
		}

		_line.clear();
		_line.resize(_text.size(), nullptr);
	}

	float xoffset = _position.x;

	float coeff = _textSize / 200.0;
	float ratio = 1.0 / _video->GetScreenRatio();

	for (size_t i = 0; i < _text.size(); ++i) {
		auto glyph = _textHandler->GetGlyph(_text[i]);

		if (glyph.HasTexture) {
			if (!_textUpdated) {
				_line[i] = new Rectangle();
			}

			float xpos = xoffset +
				coeff * glyph.Data.BearingX;
			float ypos = _position.y -
				coeff * glyph.Data.BearingY;

			_line[i]->SetRectanglePosition({
				xpos,
				ypos,
				xpos + coeff * ratio * glyph.Data.Width,
				ypos + coeff * glyph.Data.Height
			});

			if (!_textUpdated) {
				_line[i]->SetRectangleTexCoords({0, 0, 1, 1});
				_line[i]->SetRectangleDepth(_depth);
				_line[i]->SetTexture({glyph.Texture});
				_line[i]->SetColorMultiplier(_color);

				_video->RegisterRectangle(_line[i]);
			}
		}

		xoffset += coeff * ratio * glyph.Data.Advance / 64.0;
	}

	_textUpdated = true;
	_positionUpdated = true;
	_width = xoffset - _position.x;
}

void TextBox::Activate()
{
	if (!_textUpdated || !_positionUpdated) {
		Place();
	}

	for (auto rectangle : _line) {
		if (rectangle) {
			rectangle->SetDrawEnabled(true);
		}
	}
}

void TextBox::Deactivate()
{
	for (auto rectangle : _line) {
		if (rectangle) {
			rectangle->SetDrawEnabled(false);
		}
	}
}
