#include "MathHelpers.h"

double FMathHelpers::NegativeFriendlyFmod(const double A, const double B)
{
	check(B >= 0.0);
	
	if (A < 0.0)
	{
		const auto IntermediateResult = std::fmod(B - std::fmod(std::abs(A), B), B);
		return IntermediateResult;
	}

	return std::fmod(A, B);
}

int FMathHelpers::NegativeFriendlyMod(const int A, const int B)
{
	check(B >= 0);
	
	if (A < 0)
	{
		const auto IntermediateResult = (B - std::abs(A) % B) % B;
		return IntermediateResult;
	}

	return A % B;
}

FVector FMathHelpers::Lerp(const FVector& A, const FVector& B, const double T)
{
	return FVector{
		FMath::Lerp(A.X, B.X, T),
		FMath::Lerp(A.Y, B.Y, T),
		FMath::Lerp(A.Z, B.Z, T),
	};
}
