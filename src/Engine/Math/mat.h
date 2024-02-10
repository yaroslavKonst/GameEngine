#ifndef _MAT_TEMPLATE_H
#define _MAT_TEMPLATE_H

#include <cstring>

#include "vec.h"

namespace Math
{
	template<int Dim>
	struct Mat
	{
		double Data[Dim][Dim];

		Mat()
		{ }

		Mat(double diagValue)
		{
			memset(Data, 0, sizeof(Data));

			for (int i = 0; i < Dim; ++i) {
				Data[i][i] = diagValue;
			}
		}

		double* operator[](int index)
		{
			return Data[index];
		}

		const double* operator[](int index) const
		{
			return Data[index];
		}

		Mat operator+(const Mat& mat) const
		{
			Mat res;

			for (int row = 0; row < Dim; ++row) {
				for (int col = 0; col < Dim; ++col) {
					res.Data[row][col] =
						Data[row][col] +
						mat.Data[row][col];
				}
			}

			return res;
		}

		Mat operator-(const Mat& mat) const
		{
			Mat res;

			for (int row = 0; row < Dim; ++row) {
				for (int col = 0; col < Dim; ++col) {
					res.Data[row][col] =
						Data[row][col] -
						mat.Data[row][col];
				}
			}

			return res;
		}

		void operator+=(const Mat& mat)
		{
			for (int row = 0; row < Dim; ++row) {
				for (int col = 0; col < Dim; ++col) {
					Data[row][col] += mat.Data[row][col];
				}
			}
		}

		void operator-=(const Mat& mat)
		{
			for (int row = 0; row < Dim; ++row) {
				for (int col = 0; col < Dim; ++col) {
					Data[row][col] -= mat.Data[row][col];
				}
			}
		}

		Mat operator*(const Mat& mat) const
		{
			Mat res;

			for (int row = 0; row < Dim; ++row) {
				for (int col = 0; col < Dim; ++col) {
					double sum = 0;

					for (int i = 0; i < Dim; ++i) {
						sum += Data[row][i] *
							mat.Data[i][col];
					}

					res.Data[row][col] = sum;
				}
			}

			return res;
		}

		void operator*=(const Mat& mat)
		{
			double res[Dim][Dim];

			for (int row = 0; row < Dim; ++row) {
				for (int col = 0; col < Dim; ++col) {
					double sum = 0;

					for (int i = 0; i < Dim; ++i) {
						sum += Data[row][i] *
							mat.Data[i][col];
					}

					res[row][col] = sum;
				}
			}

			memcpy(Data, res, sizeof(Data));
		}

		Vec<Dim> operator*(const Vec<Dim>& vec) const
		{
			Vec<Dim> res;

			for (int row = 0; row < Dim; ++row) {
				double sum = 0;

				for (int col = 0; col < Dim; ++col) {
					sum += Data[row][col] * vec[col];
				}

				res[row] = sum;
			}

			return res;
		}
	};
}

#endif
