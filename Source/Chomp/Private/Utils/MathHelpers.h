#pragma once

class MathHelpers
{
public:
	// A version of std::fmod that treats negative numbers the way they should be treated.
	static double NotStupidFmod(const double A, const double B);
};
