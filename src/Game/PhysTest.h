#ifndef _PHYS_TEST_H
#define _PHYS_TEST_H

#include "global.h"

namespace PhysTest
{
	class Surface : public Model, public PhysicalObject
	{
	public:
		Loader::VertexData Geometry;

		Surface()
		{
			Geometry.Vertices = {
				{-2, -2, 1},
				{-2, 2, 1},
				{2, -2, 1},
				{2, 2, 1}
			};

			Geometry.Normals = {
				{0, 0, 1},
				{0, 0, 1},
				{0, 0, 1},
				{0, 0, 1}
			};

			Geometry.TexCoords = {
				{0.5, 0.5},
				{0.5, 0.5},
				{0.5, 0.5},
				{0.5, 0.5}
			};

			Geometry.MatrixIndices = {0, 0, 0, 0};

			Geometry.Indices = {0, 2, 1, 1, 2, 3};
			Geometry.Instances = {Math::Mat<4>(1.0)};
		}
	private:
	};

	class DynamicObject : public SoftObject, public Actor
	{
	public:
		DynamicObject(Common common);
		~DynamicObject();

		void Tick() override;

	private:
		Common _common;

		uint32_t _cubeModel;
		uint32_t _cubeTexture;

		std::vector<Model*> _cubes;
	};

	class PhysTest
	{
	public:
		PhysTest(Common common);
		~PhysTest();

	private:
		Common _common;

		Surface _surface;
		DynamicObject _object;
	};
}

#endif
