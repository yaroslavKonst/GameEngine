#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <vector>

#include "../VideoEngine/glm.h"

class Animation
{
public:
	struct TimePoint
	{
		glm::vec3 Position;
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

	glm::mat4 GetTransform(float time, glm::mat4 matrix = glm::mat4(1.0f));
	glm::mat4 Step(glm::mat4 matrix = glm::mat4(1.0f));

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

private:
	std::vector<TimePoint> _sequence;
	std::vector<float> _timeValues;

	float _current;
	float _step;
	bool _cycle;
	bool _twoPos;
};

#endif
