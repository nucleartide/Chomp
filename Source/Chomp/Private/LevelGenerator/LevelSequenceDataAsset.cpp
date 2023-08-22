#include "LevelSequenceDataAsset.h"

TArray<ULevelDataAsset*> ULevelSequenceDataAsset::LastThree()
{
	TArray<ULevelDataAsset*> Triplet;
	for (auto i = LevelSequence.Num() - 3; i < LevelSequence.Num(); i++)
	{
		if (i >= 0)
		{
			Triplet.Add(LevelSequence[i]);
		}
	}

	return Triplet;
}
