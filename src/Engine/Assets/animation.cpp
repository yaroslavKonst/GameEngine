#include "animation.h"

#include <algorithm>

#include "../Math/transform.h"
#include "../Logger/logger.h"

Math::Mat<4> Animation::GetTransform(float time, Math::Mat<4> matrix)
{
	int index = 0;

	while (!(_timeValues[index] <= time && _timeValues[index + 1] > time)) {
		++index;
	}

	float duration = _timeValues[index + 1] - _timeValues[index];
	float coeff = (time - _timeValues[index]) / duration;

	auto& elem = _sequence[index];
	auto& elemNext = _sequence[index + 1];

	Math::Vec<3> pos =
		elem.Position * (1.0 - coeff) + elemNext.Position * coeff;

	float rotX =
		elem.RotationX * (1.0f - coeff) + elemNext.RotationX * coeff;

	float rotY =
		elem.RotationY * (1.0f - coeff) + elemNext.RotationY * coeff;

	float rotZ =
		elem.RotationZ * (1.0f - coeff) + elemNext.RotationZ * coeff;

	matrix = Math::Translate(pos) * matrix;
	matrix *= Math::Rotate(rotZ, {0, 0, 1}, Math::Degrees);
	matrix *= Math::Rotate(rotX, {1, 0, 0}, Math::Degrees);
	matrix *= Math::Rotate(rotY, {0, 1, 0}, Math::Degrees);

	return matrix;
}

Math::Mat<4> Animation::Step(Math::Mat<4> matrix)
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
