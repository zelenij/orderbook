#include <cmath>

#include "Common.hxx"

namespace trading 
{
	bool essentiallyEqual(double a, double b, double epsilon)
	{
		return std::abs(a - b) <= ( (std::abs(a) > std::abs(b) ? std::abs(b) : std::abs(a)) * epsilon);
	}
}
