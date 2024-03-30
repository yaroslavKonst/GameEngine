#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <vector>

#include "../Math/vec.h"
#include "../Math/mat.h"

class Animation
{
public:
	struct TimePoint
	{
		Math::Vec<3> Position;
		float RotationX;
		float RotationY;
		float RotationZ;
	};

	Animation()
	{
		_current = 0;
		_step = 0;
		_cycle = false;
		_twoPos = false;
	}

	Math::Mat<4> GetTransform(
		float time,
		Math::Mat<4> matrix = Math::Mat<4>(1.0f));
	Math::Mat<4> Step(Math::Mat<4> matrix = Math::Mat<4>(1.0f));

	void SetStep(float value)
	{
		_step = value;
	}

	void SetCycle(bool value)
	{
		_cycle = value;
	}

	void SetTwoPos(bool value)
	{
		_twoPos = value;
	}

	void SetTimePoints(const std::vector<TimePoint>& sequence)
	{
		_sequence = sequence;
	}

	void SetTimeValues(const std::vector<float>& sequence)
	{
		_timeValues = sequence;
	}

	float GetMaxTime()
	{
		return _timeValues.back();
	}

private:
	std::vector<TimePoint> _sequence;
	std::vector<float> _timeValues;

	float _current;
	float _step;
	bool _cycle;
	bool _twoPos;
};

#endif
