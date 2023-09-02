#ifndef _COMMON_H
#define _COMMON_H

typedef std::map<std::string, uint32_t> Library;

struct Coord2D
{
	int32_t X;
	int32_t Y;

	bool operator<(const Coord2D& coord) const
	{
		if (X != coord.X) {
			return X < coord.X;
		}

		return Y < coord.Y;
	}
};

#endif
