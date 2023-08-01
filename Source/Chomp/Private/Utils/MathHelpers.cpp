#include "MathHelpers.h"

double MathHelpers::NotStupidFmod(const double A, const double B)
{
	if (A < 0.0)
	{
		const auto IntermediateResult = std::fmod(B - std::fmod(std::abs(A), B), B);
		return IntermediateResult;
	}

	return std::fmod(A, B);
}
