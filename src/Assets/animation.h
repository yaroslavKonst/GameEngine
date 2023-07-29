#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

class Animation
{
public:
	struct TimePoint
	{
		glm::vec3 Position;
		float RotationX;
		float RotationY;
		float RotationZ;

		glm::vec3 Alignment;
		float AlignmentForce;
	};

	Animation()
	{
		_current = 0;
		_step = 0;
		_cycle = false;
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
};

#endif
