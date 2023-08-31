#pragma once

class FMathHelpers
{
public:
	// A modulo operation that also operates upon negative numbers.
	//
	// Example: NegativeFriendlyFmod(-4.0, 3.0) ~= 2.0
	static double NegativeFriendlyFmod(const double A, const double B);

	// A modulo operation that also operates upon negative numbers.
	//
	// Example: NegativeFriendlyMod(-4, 3) == 2
	static int NegativeFriendlyMod(const int A, const int B);

	static FVector Lerp(const FVector& A, const FVector& B, const double T);
};
