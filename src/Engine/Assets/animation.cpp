#include "animation.h"

#include "../Logger/logger.h"

glm::mat4 Animation::GetTransform(float time, glm::mat4 matrix)
{
	int index = 0;

	while (!(_timeValues[index] <= time && _timeValues[index + 1] > time)) {
		++index;
	}

	float duration = _timeValues[index + 1] - _timeValues[index];
	float coeff = (time - _timeValues[index]) / duration;

	auto& elem = _sequence[index];
	auto& elemNext = _sequence[index + 1];

	glm::vec3 pos =
		elem.Position * (1.0f - coeff) + elemNext.Position * coeff;

	float rotX =
		elem.RotationX * (1.0f - coeff) + elemNext.RotationX * coeff;

	float rotY =
		elem.RotationY * (1.0f - coeff) + elemNext.RotationY * coeff;

	float rotZ =
		elem.RotationZ * (1.0f - coeff) + elemNext.RotationZ * coeff;

	matrix = glm::translate(matrix, pos);
	matrix = glm::rotate(matrix, glm::radians(rotZ), glm::vec3(0, 0, 1));
	matrix = glm::rotate(matrix, glm::radians(rotX), glm::vec3(1, 0, 0));
	matrix = glm::rotate(matrix, glm::radians(rotY), glm::vec3(0, 1, 0));

	return matrix;
}

glm::mat4 Animation::Step(glm::mat4 matrix)
{
	auto res = GetTransform(_current, matrix);

	_current += _step;

	if (_current >= _timeValues.back() || _current < _timeValues.front()) {
		if (!_twoPos) {
			_current = _timeValues.front();
		} else {
			_current = std::clamp(
				_current,
				_timeValues.front(),
				_timeValues.back());
		}

		if (!_cycle || _twoPos) {
			_step = 0;
		}
	}

	return res;
}
