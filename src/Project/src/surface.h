#ifndef SURFACE_H
#define SURFACE_H

#include <cmath>

class Surface
{
public:
	static double Height(double x, double y)
	{
		x /= 30.0;
		y /= 30.0;

		double ef = exp(-x * x - y * y);

		double f1 = ef * 5.0 + exp(-ef * 160.0) * 15.0;

		// x = y
		double f2 =
			std::min<double>(exp(fabs(pow(x - y, 6))) * 5.0, 15.0);

		return std::min<double>(f1, f2);
	}
};

#endif
