#include "LocalStorageSubsystem.h"
#include "ChompSaveGame.h"

void ULocalStorageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ChompSaveGame = UChompSaveGame::Load();
}

void ULocalStorageSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ChompSaveGame->Save();
}

UChompSaveGame* ULocalStorageSubsystem::GetSaveGame() const
{
	return ChompSaveGame;
}
