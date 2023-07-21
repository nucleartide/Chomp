#include "Controllers/ChompPlayerController.h"
#include "Pawns/ChompPawn.h"
#include "Pawns/MovablePawn.h"
#include "Utils/Debug.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "ChompGameState.h"
#include "Engine/World.h"

AChompPlayerController::AChompPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Disable this. We want to manage the active camera ourselves.
    bAutoManageActiveCameraTarget = false;
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
    auto MovablePawn = GetPawn<AMovablePawn>();
    if (!MovablePawn)
        return;

    auto World = GetWorld();
    check(World);

    // Check whether game is playing.
    auto IsGamePlaying = World->GetGameState<AChompGameState>()->GetEnum() == EChompGameState::Playing;
    if (!IsGamePlaying)
        return;

    TArray<FHitResult> HitResults;
    FVector StartPos{0.0f, -100.0f, 0.0f};
    FVector TargetPos{100.0f, -100.0f, 0.0f};
	auto ActorSphere = FCollisionShape::MakeSphere(49.5f);
    World->SweepMultiByChannel(
        HitResults,
        StartPos,
        TargetPos,
        FQuat::Identity,
        ECC_Visibility,
        ActorSphere
    );

    if (!IsTargetTileSet)
    {
        // Then attempt to set the target tile.
        auto LevelInstance = ULevelLoader::GetInstance(Level);
        auto ActorLocation = MovablePawn->GetActorLocation();
        auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();
        auto IsPassable = LevelInstance->ComputeTargetTile(World, MovablePawn, CurrentMoveDirection, TagsToCollideWith, TargetTile);
        if (IsPassable)
            IsTargetTileSet = true;
        else if (IntendedMoveDirection.IsNonZero() && IntendedMoveDirection != CurrentMoveDirection)
            // Then try again with intended direction.
            CurrentMoveDirection = IntendedMoveDirection;
    }

    // If there is a target tile,
    if (IsTargetTileSet)
    {
        // Then move toward the target tile.
        if (MovablePawn->MoveTowardsPoint(TargetTile, CurrentMoveDirection, DeltaTime))
        {
            DEBUG_LOG(TEXT("Exceeded destination point."));
        }
    }

#if false
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
#endif
}

void AChompPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Bind input axes.
    InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
    InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);
}
