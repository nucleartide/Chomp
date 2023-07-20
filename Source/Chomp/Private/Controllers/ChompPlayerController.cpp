#include "Controllers/ChompPlayerController.h"
#include "Pawns/ChompPawn.h"
#include "Utils/Debug.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "ChompGameState.h"

AChompPlayerController::AChompPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AChompPlayerController::OnMoveVertical(float Input)
{
    IntendedMoveDirection.X = FGenericPlatformMath::RoundToInt(Input);
}

void AChompPlayerController::OnMoveHorizontal(float Input)
{
    IntendedMoveDirection.Y = FGenericPlatformMath::RoundToInt(Input);
}

void AChompPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get reference to pawn.
    auto ChompPawn = GetPawn<AChompPawn>();
    if (!ChompPawn)
        return;

    // Check whether game is playing.
    auto IsGamePlaying = GetWorld()->GetGameState<AChompGameState>()->GetEnum() == EChompGameState::Playing;
    if (!IsGamePlaying)
        return;

    // If there is no target tile,
    // TODO: check if currentmove != intendedmove, and if so set the target anew as well.
    if (!IsTargetTileSet)
    {
        // Attempt to set the target tile.
        auto LevelInstance = ULevelLoader::GetInstance(Level);
        auto ActorLocation = ChompPawn->GetActorLocation();
        auto ExcludedEntities = ChompPawn->GetExcludedEntities();
        auto IsPassable = LevelInstance->ComputeTargetTile(ActorLocation, CurrentMoveDirection, ExcludedEntities, TargetTile);
        if (IsPassable)
            IsTargetTileSet = true;

        // Note that CurrentMoveDirection is always moving.
        // ...
    }

    // If there is a target tile,
    if (IsTargetTileSet)
    {
        // Move toward the target tile.
        if (ChompPawn->MoveTowardsPoint(TargetTile, DeltaTime))
        {
            // Then, check if the latest intended move from the target grid position would be legal.
            if (true)
            {
                // If so, re-align player to the target grid position + intended dir * delta.
                // ...
            }
            else
            {
                // Re-align player to the target grid position.
                // ...
            }

            // Finally, update current move direction from the intended.
            // Note that CurrentMoveDirection is always moving.
            // ...

            // And unset the target tile, since we've reached it.
            IsTargetTileSet = false;
        }
    }
}

void AChompPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Bind input axes.
    InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
    InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);

    // Disable this. We want to manage the active camera ourselves.
    bAutoManageActiveCameraTarget = false;
}
