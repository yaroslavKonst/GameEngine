#include "TextBox.h"

#include "../../Logger/logger.h"

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
	_text = Text::DecodeUTF8(text);
	_textUpdated = false;
}

void TextBox::SetPosition(float x, float y, Alignment alignment)
{
	_position = {x, y};
	_alignment = alignment;
	_positionUpdated = false;
}

void TextBox::SetTextSize(float size)
{
	_textSize = size;
	_positionUpdated = false;
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
	float yoffset = _position.y;

	uint32_t medianHeight = _textHandler->GetMedianGlyphHeight();

	float coeff = _textSize / medianHeight;

	for (size_t i = 0; i < _text.size(); ++i) {
		if (_text[i] == '\n') {
			xoffset = _position.x;
			yoffset += _textSize * 1.5;
			continue;
		}

		auto glyph = _textHandler->GetGlyph(_text[i]);

		if (glyph.HasTexture) {
			if (!_textUpdated) {
				_line[i] = new Rectangle();
			}

			float xpos = xoffset +
				coeff * glyph.Data.BearingX;
			float ypos = yoffset -
				coeff * glyph.Data.BearingY;

			_line[i]->RectangleParams.Position = {
				xpos,
				ypos,
				xpos + coeff * glyph.Data.Width,
				ypos + coeff * glyph.Data.Height
			};

			_line[i]->RectangleParams.Depth = _depth;
			_line[i]->DrawParams.ColorMultiplier = _color;

			if (!_textUpdated) {
				_line[i]->RectangleParams.TexCoords =
					{0, 0, 1, 1};
				_line[i]->RectangleParams.Texture =
					glyph.Texture;

				_video->RegisterRectangle(_line[i]);
			}
		} else {
			_line[i] = nullptr;
		}

		xoffset += coeff * glyph.Data.Advance / 64.0;
	}

	_textUpdated = true;
	_positionUpdated = true;
	_width = xoffset - _position.x;

	if (_alignment != Alignment::Left) {
		float shift;

		if (_alignment == Alignment::Center) {
			shift = -_width / 2.0;
		} else if (_alignment == Alignment::Right) {
			shift = -_width;
		} else {
			return;
		}

		for (auto rectangle : _line) {
			if (rectangle) {
				rectangle->RectangleParams.Position[0] += shift;
				rectangle->RectangleParams.Position[2] += shift;
			}
		}
	}
}

void TextBox::Activate()
{
	if (!_textUpdated || !_positionUpdated) {
		Place();
	}

	for (auto rectangle : _line) {
		if (rectangle) {
			rectangle->DrawParams.Enabled = true;
		}
	}
}

void TextBox::Deactivate()
{
	for (auto rectangle : _line) {
		if (rectangle) {
			rectangle->DrawParams.Enabled = false;
		}
	}
}
