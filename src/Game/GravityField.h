#ifndef _GRAVITY_FIELD_H
#define _GRAVITY_FIELD_H

class GravityField
{
public:
	static constexpr double G_CONST = 6.67430e-11;

	struct Object
	{
		double Mass;
		Math::Vec<3> Position;
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

	Math::Vec<3> operator()(const Math::Vec<3>& pos)
	{
		Math::Vec<3> res(0.0);

		for (auto& object : _objects) {
			Math::Vec<3> delta = object.Position - pos;

			res += delta.Normalize() * object.Mass /
				pow(delta.Length(), 2.0);
		}

		return res;
	}

private:
	std::vector<Object> _objects;
};

#endif
