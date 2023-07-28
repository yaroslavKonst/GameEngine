#ifndef _LIGHT_H
#define _LIGHT_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

class Light
{
public:
	enum class Type
	{
		Point = 0,
		Spot = 1,
		Direct = 2
	};

	Light()
	{
		_active = false;
	}

	virtual ~Light()
	{
	}

	virtual const glm::vec3& GetLightPosition()
	{
		return _position;
	}

	virtual void SetLightPosition(const glm::vec3& value)
	{
		_position = value;
	}

	virtual const glm::vec3& GetLightColor()
	{
		return _color;
	}

	virtual void SetLightColor(const glm::vec3& value)
	{
		_color = value;
	}

	virtual const glm::vec3& GetLightDirection()
	{
		return _direction;
	}

	virtual void SetLightDirection(const glm::vec3& value)
	{
		_direction = value;
	}

	virtual Type GetLightType()
	{
		return _type;
	}

	virtual void SetLightType(Type value)
	{
		_type = value;
	}

	virtual float GetLightAngle()
	{
		return _angle;
	}

	virtual void SetLightAngle(float value)
	{
		_angle = value;
	}

	virtual float GetLightAngleFade()
	{
		return _angleFade;
	}

	virtual void SetLightAngleFade(float value)
	{
		_angleFade = value;
	}

	virtual bool IsLightActive()
	{
		return _active;
	}

	virtual void SetLightActive(bool value)
	{
		_active = value;
	}

private:
	glm::vec3 _position;
	glm::vec3 _color;
	glm::vec3 _direction;
	Type _type;
	float _angle;
	float _angleFade;

	bool _active;
};

#endif
