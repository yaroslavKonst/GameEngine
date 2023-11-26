#ifndef _GRAVITY_FIELD_H
#define _GRAVITY_FIELD_H

class GravityField
{
public:
	static constexpr float G_CONST = 6.67430e-11;

	struct Object
	{
		float Mass;
		glm::vec3 Position;
	};

	GravityField()
	{
	}

	~GravityField()
	{
	}

	void AddObject(const Object& object)
	{
		_objects.push_back(object);
	}

	glm::vec3 operator()(const glm::vec3& pos)
	{
		glm::vec3 res = {0, 0, 0};

		for (auto& object : _objects) {
			glm::vec3 delta = object.Position - pos;

			res += glm::normalize(delta) * object.Mass /
				powf(glm::length(delta), 2);
		}

		return res;
	}

private:
	std::vector<Object> _objects;
};

#endif
