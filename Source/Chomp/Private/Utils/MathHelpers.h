#pragma once

class FMathHelpers
{
public:
	// A modulo operation that also operates upon negative numbers.
	//
	// Example: NegativeFriendlyFmod(-4.0, 3.0) ~= 2.0
	static double NegativeFriendlyFmod(const double A, const double B);
};
