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

    // Sanity check that World & GameState aren't null.
    auto World = GetWorld();
    check(World);
    auto GameState = World->GetGameState<AChompGameState>();
    check(GameState);

    // Early return if game is not playing.
    auto IsGamePlaying = GameState->GetEnum() == EChompGameState::Playing;
    if (!IsGamePlaying)
        return;

    // Early return if pawn is dead.
    auto MovablePawn = GetPawn<AMovablePawn>();
    if (!MovablePawn)
        return;

    UpdateIntendedMoveDirection();

    auto LevelInstance = ULevelLoader::GetInstance(Level);
    UpdateCurrentMoveDirectionAndTarget(
        CurrentMoveDirection,
        Target,
        IntendedMoveDirection,
        World,
        MovablePawn,
        LevelInstance,
        DeltaTime);
}

void AChompPlayerController::UpdateCurrentMoveDirectionAndTarget(
    FGridLocation &CurrentMoveDirection,
    FComputeTargetTileResult &Target,
    FGridLocation IntendedMoveDirection,
    UWorld *World,
    AMovablePawn *MovablePawn,
    ULevelLoader *LevelInstance,
    float DeltaTime)
{
    auto ActorLocation = MovablePawn->GetActorLocation();
    auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();

    if (!Target.IsValid)
    {
        Target = LevelInstance->ComputeTargetTile(World, ActorLocation, CurrentMoveDirection, TagsToCollideWith);
        // CurrentMoveDirection remains the same.
    }

    if (IntendedMoveDirection.IsNonZero() && (!Target.IsValid || CurrentMoveDirection != IntendedMoveDirection))
    {
        auto Result = LevelInstance->ComputeTargetTile(World, ActorLocation, IntendedMoveDirection, TagsToCollideWith);
        if (Result.IsValid)
        {
            Target = Result;
            CurrentMoveDirection = IntendedMoveDirection;
        }
    }

    if (Target.IsValid)
    {
        auto MovementResult = MovablePawn->MoveTowardsPoint(Target.Tile, CurrentMoveDirection, DeltaTime);
        if (MovementResult.MovedPastTarget)
        {
            // Check if next move from target is valid.
            auto TargetWorldPos = LevelInstance->GridToWorld(Target.Tile);
            FVector TargetWorldVec{TargetWorldPos.X, TargetWorldPos.Y, 0.0f};
            auto Dir = IntendedMoveDirection.IsNonZero() ? IntendedMoveDirection : CurrentMoveDirection;
            auto Result = LevelInstance->ComputeTargetTile(World, TargetWorldVec, Dir, TagsToCollideWith);

            // Update our actor's state depending on the result.
            Target = Result;
            if (Result.IsValid) CurrentMoveDirection = Dir;

            // Finally, update our actor's position depending on the result.
            if (Result.IsValid)
            {
                FVector NewLocation{
                    TargetWorldPos.X + Dir.X * MovementResult.AmountMovedPast,
                    TargetWorldPos.Y + Dir.Y * MovementResult.AmountMovedPast,
                    0.0f};
                MovablePawn->SetActorLocation(NewLocation);
            }
            else
            {
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
        Target.IsValid = false;
        CurrentMoveDirection.X = InitialMoveDirection.X;
        CurrentMoveDirection.Y = InitialMoveDirection.Y;
    }
}
