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

// intended key should only update if new intended key is non-zero
// else, it clears after 1s

void AChompPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    auto World = GetWorld();
    check(World);
    if (VerticalAxis != 0.0f || HorizontalAxis != 0.0f)
    {
        // Update intended move direction using raw input.
        IntendedMoveDirection.X = FGenericPlatformMath::RoundToInt(VerticalAxis);
        IntendedMoveDirection.Y = FGenericPlatformMath::RoundToInt(HorizontalAxis);
        TimeOfLastIntendedDirUpdate = World->GetRealTimeSeconds();
    }
    else if (World->GetRealTimeSeconds() > TimeOfLastIntendedDirUpdate + 1.0f) // Clear input after 1s.
    {
        IntendedMoveDirection.X = 0;
        IntendedMoveDirection.Y = 0;
    }

    // Get reference to pawn.
    auto MovablePawn = GetPawn<AMovablePawn>();
    if (!MovablePawn)
        return;

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
        auto IsPassable = LevelInstance->ComputeTargetTile(World, ActorLocation, CurrentMoveDirection, TagsToCollideWith, TargetTile);
        if (IsPassable)
            IsTargetTileSet = true;
        else if (IntendedMoveDirection.IsNonZero())
            CurrentMoveDirection = IntendedMoveDirection;
    }
    else if (IsTargetTileSet && IntendedMoveDirection.IsNonZero() && IntendedMoveDirection != CurrentMoveDirection)
    {
        auto LevelInstance = ULevelLoader::GetInstance(Level);
        auto ActorLocation = MovablePawn->GetActorLocation();
        auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();
        auto IsPassable = LevelInstance->ComputeTargetTile(World, ActorLocation, IntendedMoveDirection, TagsToCollideWith, TargetTile);
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
            DEBUG_LOG(TEXT("Exceeded destination point."));

            // Check if the latest intended move from the target grid position would be legal.
            auto LevelInstance = ULevelLoader::GetInstance(Level);
            auto TargetWorldPos = LevelInstance->GridToWorld(TargetTile);
            FVector TargetWorldVec{TargetWorldPos.X, TargetWorldPos.Y, 0.0f};
            auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();
            auto Dir = IntendedMoveDirection.IsNonZero() ? IntendedMoveDirection : CurrentMoveDirection;
            auto IsPassable = LevelInstance->ComputeTargetTile(World, TargetWorldVec, Dir, TagsToCollideWith, TargetTile);
            if (IsPassable)
            {
                // Set the actor's position to the TargetWorldPos + IntendedMoveDirection * MovementResult.AmountMovedPast.
                FVector NewLocation{
                    TargetWorldPos.X + IntendedMoveDirection.X * MovementResult.AmountMovedPast,
                    TargetWorldPos.Y + IntendedMoveDirection.Y * MovementResult.AmountMovedPast,
                    0.0f};
                MovablePawn->SetActorLocation(NewLocation);
                if (IntendedMoveDirection.IsNonZero())
                    CurrentMoveDirection = IntendedMoveDirection;
            }
            else
            {
                // Re-align player to target grid position.
                FVector Pos{TargetWorldPos.X, TargetWorldPos.Y, 0.0f};
                MovablePawn->SetActorLocation(Pos);

                // And unset the target tile, since we've reached it.
                IsTargetTileSet = false;
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

    auto World = GetWorld();
    check(World);

    World->GetGameState<AChompGameState>()->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AChompPlayerController::HandleGameRestarted);
}

void AChompPlayerController::HandleGameRestarted(EChompGameState OldState, EChompGameState NewState)
{
    check(OldState != NewState);
    if (NewState == EChompGameState::Playing)
    {
        IsTargetTileSet = false;
        // reset this to original value
        CurrentMoveDirection.X = InitialMoveDirection.X;
        CurrentMoveDirection.Y = InitialMoveDirection.Y;
    }
}
