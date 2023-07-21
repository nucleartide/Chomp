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
    VerticalAxis = Input;
}

void AChompPlayerController::OnMoveHorizontal(float Input)
{
    HorizontalAxis = Input;
}

void AChompPlayerController::UpdateIntendedMoveDirection()
{
    auto World = GetWorld();
    check(World);

    if (VerticalAxis != 0.0f || HorizontalAxis != 0.0f)
    {
        // Update intended move direction using raw input.
        IntendedMoveDirection.X = FGenericPlatformMath::RoundToInt(VerticalAxis);
        IntendedMoveDirection.Y = FGenericPlatformMath::RoundToInt(HorizontalAxis);
        TimeOfLastIntendedDirUpdate = World->GetRealTimeSeconds();
    }
    else if (World->GetRealTimeSeconds() > TimeOfLastIntendedDirUpdate + TimeForIntendedDirectionToLast)
    {
        // Clear input if there's been no new input for awhile (this is configurable).
        IntendedMoveDirection.X = 0;
        IntendedMoveDirection.Y = 0;
    }
}

void AChompPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Sanity check that world isn't null.
    auto World = GetWorld();
    check(World);

    // Early return if pawn is dead.
    auto MovablePawn = GetPawn<AMovablePawn>();
    if (!MovablePawn)
        return;

    // Early return if game is not playing.
    auto IsGamePlaying = World->GetGameState<AChompGameState>()->GetEnum() == EChompGameState::Playing;
    if (!IsGamePlaying)
        return;

    // Process input.
    UpdateIntendedMoveDirection();

    // Some values we need for later.
    auto LevelInstance = ULevelLoader::GetInstance(Level);
    auto ActorLocation = MovablePawn->GetActorLocation();
    auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();

    // If target tile isn't set,
    if (!IsTargetTileSet)
    {
        // Then attempt to set the target tile.
        IsTargetTileSet = LevelInstance->ComputeTargetTile(World, ActorLocation, CurrentMoveDirection, TagsToCollideWith, TargetTile);
        if (!IsTargetTileSet && IntendedMoveDirection.IsNonZero()) CurrentMoveDirection = IntendedMoveDirection;
    }
    // If the intended move direction is non-zero and different,
    else if (IntendedMoveDirection.IsNonZero() && IntendedMoveDirection != CurrentMoveDirection)
    {
        // Then attempt to set the target tile.
        auto IsPassable = LevelInstance->ComputeTargetTile(World, ActorLocation, IntendedMoveDirection, TagsToCollideWith, TargetTile);

        // If successful, update the current movement direction.
        if (IsPassable)
            CurrentMoveDirection = IntendedMoveDirection;
    }

    // If there is a target tile,
    if (IsTargetTileSet)
    {
        // Then move toward the target tile.
        auto MovementResult = MovablePawn->MoveTowardsPoint(TargetTile, CurrentMoveDirection, DeltaTime);
        if (MovementResult.MovedPastTarget)
        {
            // Check if next move from the target grid position is legal.
            auto TargetWorldPos = LevelInstance->GridToWorld(TargetTile);
            FVector TargetWorldVec{TargetWorldPos.X, TargetWorldPos.Y, 0.0f};
            auto Dir = IntendedMoveDirection.IsNonZero() ? IntendedMoveDirection : CurrentMoveDirection;
            IsTargetTileSet = LevelInstance->ComputeTargetTile(World, TargetWorldVec, Dir, TagsToCollideWith, TargetTile);

            // Depending on whether the next move is legal, update the actor's position accordingly.
            if (IsTargetTileSet)
            {
                // Set the actor's position to the TargetWorldPos + IntendedMoveDirection * MovementResult.AmountMovedPast.
                FVector NewLocation{
                    TargetWorldPos.X + IntendedMoveDirection.X * MovementResult.AmountMovedPast,
                    TargetWorldPos.Y + IntendedMoveDirection.Y * MovementResult.AmountMovedPast,
                    0.0f};
                MovablePawn->SetActorLocation(NewLocation);

                // Update the current move direction if the intended direction is non-zero.
                if (IntendedMoveDirection.IsNonZero()) CurrentMoveDirection = IntendedMoveDirection;
            }
            else
            {
                // Re-align player to target grid position.
                FVector NewLocation{TargetWorldPos.X, TargetWorldPos.Y, 0.0f};
                MovablePawn->SetActorLocation(NewLocation);
            }
        }
    }
}

void AChompPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Bind input axes.
    InputComponent->BindAxis("Move Forward / Backward", this, &AChompPlayerController::OnMoveVertical);
    InputComponent->BindAxis("Move Right / Left", this, &AChompPlayerController::OnMoveHorizontal);

    // Bind restart handler.
    auto World = GetWorld();
    check(World);
    World->GetGameState<AChompGameState>()->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AChompPlayerController::HandleGameRestarted);
}

void AChompPlayerController::HandleGameRestarted(EChompGameState OldState, EChompGameState NewState)
{
    check(OldState != NewState);
    if (NewState == EChompGameState::Playing)
    {
        // If we are restarting, reset some internal state around the selected target tile + move direction.
        IsTargetTileSet = false;
        CurrentMoveDirection.X = InitialMoveDirection.X;
        CurrentMoveDirection.Y = InitialMoveDirection.Y;
    }
}
