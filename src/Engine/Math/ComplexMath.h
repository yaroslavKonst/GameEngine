#ifndef _COMPLEX_MATH_H
#define _COMPLEX_MATH_H

#include <cmath>
#include <vector>

class Complex
{
public:
	Complex()
	{
		_re = 0;
		_im = 0;
	}

	Complex(float value)
	{
		_re = value;
		_im = 0;
	}

	Complex(float re, float im)
	{
		_re = re;
		_im = im;
	}

	Complex(const Complex& value)
	{
		_re = value._re;
		_im = value._im;
	}

	Complex& operator=(const Complex& value)
	{
		_re = value._re;
		_im = value._im;

		return *this;
	}

	bool operator==(const Complex& value) const
	{
		return _re == value._re && _im == value._im;
	}

	const float& Re() const
	{
		return _re;
	}

	const float& Im() const
	{
		return _im;
	}

	float& Re()
	{
		return _re;
	}

	float& Im()
	{
		return _im;
	}

	float Mod() const
	{
		return sqrtf(_re * _re + _im * _im);
	}

	float Arg() const
	{
		if (_re > 0) {
			return atanf(_im / _re);
		} else if (_re < 0) {
			float angle = atanf(_im / _re);

			if (angle > 0) {
				angle -= M_PI;
			} else {
				angle += M_PI;
			}

			return angle;
		} else if (_im > 0) {
			return M_PI / 2.0;
		} else if (_im < 0) {
			return -M_PI / 2.0;
		} else {
			return 0;
		}
	}

	Complex operator+(const Complex& value) const
	{
		return Complex(_re + value._re, _im + value._im);
	}

	Complex operator-(const Complex& value) const
	{
		return Complex(_re - value._re, _im - value._im);
	}

	Complex operator*(const Complex& value) const
	{
		return Complex(
			_re * value._re - _im * value._im,
			_im * value._re + _re * value._im);
	}

	void operator+=(const Complex& value)
	{
		_re += value._re;
		_im += value._im;
	}

	void operator-=(const Complex& value)
	{
		_re -= value._re;
		_im -= value._im;
	}

	void operator*=(const Complex& value)
	{
		float re = _re * value._re - _im * value._im;
		float im = _im * value._re + _re * value._im;

		_re = re;
		_im = im;
	}

private:
	float _re;
	float _im;
};

inline std::vector<Complex> DFT(std::vector<Complex> array)
{
	size_t len = array.size();
	std::vector<Complex> result(len);

	#pragma omp parallel for
	for (size_t outIdx = 0; outIdx < len; ++outIdx) {
		Complex sum;

		for (size_t inIdx = 0; inIdx < len; ++inIdx) {
			float val = 2.0 * M_PI * outIdx * inIdx / len;
			sum += array[inIdx] * Complex(cosf(val), -sinf(val));
		}

		result[outIdx] = sum;
	}

	return result;
}

inline std::vector<Complex> IDFT(std::vector<Complex> array)
{
	size_t len = array.size();
	std::vector<Complex> result(len);

	#pragma omp parallel for
	for (size_t outIdx = 0; outIdx < len; ++outIdx) {
		Complex sum;

		for (size_t inIdx = 0; inIdx < len; ++inIdx) {
			float val = 2.0 * M_PI * outIdx * inIdx / len;
			sum += array[inIdx] * Complex(cosf(val), sinf(val));
		}

		result[outIdx] = sum * (1.0f / len);
	}

	return result;
}

#endif
