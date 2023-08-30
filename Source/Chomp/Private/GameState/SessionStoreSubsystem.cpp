#include "SessionStoreSubsystem.h"

#include "ChompSaveGame.h"
#include "UI/LevelSymbolWidget.h"

void USessionStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ChompSaveGame = UChompSaveGame::Load();
}

void USessionStoreSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ChompSaveGame->Save();
}

UChompSaveGame* USessionStoreSubsystem::GetSaveGame() const
{
	return ChompSaveGame;
}
