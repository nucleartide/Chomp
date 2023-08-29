#include "HighScoreSubsystem.h"
#include "LevelGenerator/LevelDataAsset.h"
#include "UI/LevelSymbolWidget.h"

int UHighScoreSubsystem::GetHighScore() const
{
	return HighScore;
}

bool UHighScoreSubsystem::GetIsHighScoreNew() const
{
	return IsHighScoreNew;
}

TSubclassOf<UUserWidget> UHighScoreSubsystem::GetHighScoreBonusSymbolWidget() const
{
	if (!HighScoreLevel)
	{
		return nullptr;
	}
	
	return TSubclassOf<UUserWidget>(HighScoreLevel->BonusSymbolWidget);
}
