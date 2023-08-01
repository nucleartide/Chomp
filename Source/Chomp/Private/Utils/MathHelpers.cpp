#include "MathHelpers.h"

double MathHelpers::NotStupidFmod(const double A, const double B)
{
	if (A < 0.0)
		return B - std::fmod(std::abs(A), B);

	return std::fmod(A, B);
}
