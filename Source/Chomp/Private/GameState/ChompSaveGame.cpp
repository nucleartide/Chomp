#include "ChompSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/Debug.h"

UChompSaveGame* UChompSaveGame::Load(const FString SaveSlotName, const int UserIndex)
{
	if (const auto LoadedGame = Cast<UChompSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex)))
	{
		DEBUG_LOG(TEXT("Did successfully load. High score: %d"), LoadedGame->HighScore);
		return LoadedGame;
	}

	DEBUG_LOG(TEXT("Did not successfully load. Loading default save game."));
	const auto DefaultSaveGame = Cast<UChompSaveGame>(UGameplayStatics::CreateSaveGameObject(StaticClass()));

	// Post-conditions.
	check(DefaultSaveGame);

	return DefaultSaveGame;
}

void UChompSaveGame::Save(const FString SaveSlotName, const int UserIndex)
{
	if (!UGameplayStatics::SaveGameToSlot(this, SaveSlotName, UserIndex))
	{
		DEBUG_LOGERROR(TEXT("Failed to save game. Note that high score won't be persisted."));
	}
}

int UChompSaveGame::GetHighScore() const
{
	return HighScore;
}

void UChompSaveGame::SetHighScore(int NewHighScore)
{
	HighScore = NewHighScore;
	IsDirty = true;
}

bool UChompSaveGame::IsHighScoreNew() const
{
	return IsDirty;
}

ULevelDataAsset* UChompSaveGame::GetHighScoreLevel() const
{
	return HighScoreLevel;
}

void UChompSaveGame::SetHighScoreLevel(ULevelDataAsset* NewHighScoreLevel)
{
	HighScoreLevel = NewHighScoreLevel;
	IsDirty = true;
}
