#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "mat.h"
#include "vec.h"

namespace Math
{
	inline Mat<4> Translate(Vec<3> shift)
	{
		Mat<4> result(1.0);

		for (int i = 0; i < 3; ++i) {
			result[i][3] = shift[i];
		}

		return result;
	}

	enum AngleType
	{
		Radians,
		Degrees
	};

	inline Mat<4> Rotate(
		double angle,
		Vec<3> axis,
		AngleType angleType = Radians)
	{
		Mat<4> result(1.0);
		Vec<3> normalizedAxis = axis.Normalize();

		double radAngle;

		if (angleType == Radians) {
			radAngle = angle;
		} else {
			radAngle = angle * M_PI / 180.0;
		}

		double aSin = sin(radAngle);
		double aCos = cos(radAngle);

		Mat<3> Ax(0.0);

		Ax[1][0] = normalizedAxis[2];
		Ax[2][0] = -normalizedAxis[1];
		Ax[2][1] = normalizedAxis[0];
		Ax[0][1] = -normalizedAxis[2];
		Ax[0][2] = normalizedAxis[1];
		Ax[1][2] = -normalizedAxis[0];

		for (int row = 0; row < 3; ++row) {
			for (int col = 0; col < 3; ++col) {
				double value = 0;

				if (row == col) {
					value += aCos;
				} else {
					value += Ax[row][col] * aSin;
				}

				value += normalizedAxis[row] *
					normalizedAxis[col] * (1.0 - aCos);

				result[row][col] = value;
			}
		}

		return result;
	}

	inline Mat<4> Scale(Vec<3> coeff)
	{
		Mat<4> result(1.0);

		for (int i = 0; i < 3; ++i) {
			result[i][i] = coeff[i];
		}

		return result;
	}
}

#endif
