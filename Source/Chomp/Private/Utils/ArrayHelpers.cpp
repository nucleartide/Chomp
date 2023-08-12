#include "ArrayHelpers.h"
#include "AStar/GridLocation.h"
#include "Math/RandomStream.h"

template <typename T>
void FArrayHelpers::Randomize(TArray<T>& SomeArray)
{
	FRandomStream RandomStream;
	RandomStream.GenerateNewSeed();

	for (auto Index = SomeArray.Num() - 1; Index >= 1; --Index)
	{
		const auto SwapIndex = RandomStream.RandRange(0, Index);
		SomeArray.Swap(Index, SwapIndex);
	}
}

template <typename T>
void FArrayHelpers::Randomize(std::vector<T>& SomeArray)
{
	FRandomStream RandomStream;
	RandomStream.GenerateNewSeed();

	for (auto Index = SomeArray.size() - 1; Index >= 1; --Index)
	{
		const auto SwapIndex = RandomStream.RandRange(0, Index);

		// Swap elements.
		const auto Temp = SomeArray[Index];
		SomeArray[Index] = SomeArray[SwapIndex];
		SomeArray[SwapIndex] = Temp;
	}
}

template
void FArrayHelpers::Randomize(TArray<int>& SomeArray);

template
void FArrayHelpers::Randomize(std::vector<int>& SomeArray);

template
void FArrayHelpers::Randomize(std::vector<FGridLocation>& SomeArray);
