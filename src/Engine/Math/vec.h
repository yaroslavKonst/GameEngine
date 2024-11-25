#ifndef _VEC_TEMPLATE_H
#define _VEC_TEMPLATE_H

#include <initializer_list>
#include <cstring>
#include <cmath>

namespace Math
{
	template<int Dim>
	struct Vec
	{
		double Data[Dim];

		Vec()
		{ }

		Vec(double* values)
		{
			for (int i = 0; i < Dim; ++i) {
				Data[i] = values[i];
			}
		}

		Vec(double value)
		{
			for (int i = 0; i < Dim; ++i) {
				Data[i] = value;
			}
		}

		Vec(const Vec<Dim - 1>& vec, double value)
		{
			memcpy(Data, vec.Data, sizeof(double) * (Dim - 1));
			Data[Dim - 1] = value;
		}

		template<int N>
		Vec(const Vec<N>& vec)
		{
			if (N < Dim) {
				memcpy(Data, vec.Data, sizeof(vec.Data));
			} else {
				memcpy(Data, vec.Data, sizeof(Data));
			}
		}

		Vec(std::initializer_list<double> l)
		{
			int i = 0;

			for (double v : l) {
				Data[i] = v;
				++i;
			}
		}

		double& operator[](int index)
		{
			return Data[index];
		}

		double operator[](int index) const
		{
			return Data[index];
		}

		template<int N>
		Vec& operator=(const Vec<N>& vec)
		{
			if (N < Dim) {
				memcpy(Data, vec.Data, sizeof(vec.Data));
			} else {
				memcpy(Data, vec.Data, sizeof(Data));
			}

			return *this;
		}

		Vec& operator=(double* values)
		{
			for (int i = 0; i < Dim; ++i) {
				Data[i] = values[i];
			}

			return *this;
		}

		Vec& operator=(double value)
		{
			for (int i = 0; i < Dim; ++i) {
				Data[i] = value;
			}

			return *this;
		}

		Vec operator+(const Vec& vec) const
		{
			Vec res;

			for (int i = 0; i < Dim; ++i) {
				res.Data[i] = Data[i] + vec.Data[i];
			}

			return res;
		}

		Vec operator-(const Vec& vec) const
		{
			Vec res;

			for (int i = 0; i < Dim; ++i) {
				res.Data[i] = Data[i] - vec.Data[i];
			}

			return res;
		}

		Vec operator-() const
		{
			Vec res;

			for (int i = 0; i < Dim; ++i) {
				res.Data[i] = -Data[i];
			}

			return res;
		}

		void operator+=(const Vec& vec)
		{
			for (int i = 0; i < Dim; ++i) {
				Data[i] += vec.Data[i];
			}
		}

		void operator-=(const Vec& vec)
		{
			for (int i = 0; i < Dim; ++i) {
				Data[i] -= vec.Data[i];
			}
		}

		Vec operator*(double value) const
		{
			Vec res;

			for (int i = 0; i < Dim; ++i) {
				res.Data[i] = Data[i] * value;
			}

			return res;
		}

		Vec operator/(double value) const
		{
			Vec res;

			for (int i = 0; i < Dim; ++i) {
				res.Data[i] = Data[i] / value;
			}

			return res;
		}

		void operator*=(double value)
		{
			for (int i = 0; i < Dim; ++i) {
				Data[i] *= value;
			}
		}

		void operator/=(double value)
		{
			for (int i = 0; i < Dim; ++i) {
				Data[i] /= value;
			}
		}

		double Length() const
		{
			double sum = 0;

			for (int i = 0; i < Dim; ++i) {
				sum += pow(Data[i], 2);
			}

			return sqrt(sum);
		}

		double Dot(const Vec& vec) const
		{
			double sum = 0;

			for (int i = 0; i < Dim; ++i) {
				sum += Data[i] * vec.Data[i];
			}

			return sum;
		}

		Vec<3> Cross(const Vec<3>& vec) const
		{
			Vec<3> res;
			res[0] = Data[1] * vec.Data[2] - Data[2] * vec.Data[1],
			res[1] = Data[2] * vec.Data[0] - Data[0] * vec.Data[2],
			res[2] = Data[0] * vec.Data[1] - Data[1] * vec.Data[0];

			return res;
		}

		Vec Normalize() const
		{
			return *this / Length();
		}

		Vec Project(const Vec& vec) const
		{
			Vec normVec = vec.Normalize();
			return normVec * Dot(normVec);
		}
	};
}

#endif
