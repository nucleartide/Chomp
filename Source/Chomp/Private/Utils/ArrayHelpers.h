#pragma once

class FArrayHelpers
{
public:
	template <typename T>
	static void Randomize(TArray<T>& SomeArray);
	
	template <typename T>
	static void Randomize(std::vector<T>& SomeArray);
};
