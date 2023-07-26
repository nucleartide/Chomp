#include "Controllers/ChompPlayerController.h"
#include "Pawns/MovablePawn.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "ChompGameState.h"
#include "Engine/World.h"

AChompPlayerController::AChompPlayerController(): APlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Disable this. We want to manage the active camera ourselves.
    bAutoManageActiveCameraTarget = false;
}

void AChompPlayerController::OnMoveVertical(const float Input)
{
    VerticalAxis = Input;
}

void AChompPlayerController::OnMoveHorizontal(const float Input)
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
    if (auto IsGamePlaying = GameState->GetEnum() == EChompGameState::Playing; !IsGamePlaying)
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
    const FGridLocation& IntendedMoveDirection,
    UWorld *World,
    AMovablePawn *MovablePawn,
    const ULevelLoader *LevelInstance,
    const float DeltaTime)
{
    const auto ActorLocation = MovablePawn->GetActorLocation();
    const auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();

    if (!Target.IsValid)
    {
        Target = LevelInstance->ComputeTargetTile(World, ActorLocation, CurrentMoveDirection, TagsToCollideWith, TEXT("Player"));
        // CurrentMoveDirection remains the same.
    }

    if (IntendedMoveDirection.IsNonZero() && (!Target.IsValid || CurrentMoveDirection != IntendedMoveDirection))
    {
        if (const auto Result = LevelInstance->ComputeTargetTile(World, ActorLocation, IntendedMoveDirection, TagsToCollideWith, TEXT("Player")); Result.IsValid)
        {
            Target = Result;
            CurrentMoveDirection = IntendedMoveDirection;
        }
    }

    // [ ] funkiness occurs here when character wraps around, and the amount moved past is wonky
    // [ ] see if the same bug affects ghosts, in that the actor location is set twice. most likely not since ghosts don't wrap around
    if (Target.IsValid)
    {
        if (const auto [MovedPastTarget, AmountMovedPast] = MovablePawn->MoveTowardsPoint(Target.Tile, CurrentMoveDirection, DeltaTime, FName("Player")); MovedPastTarget)
        {
            // Check if next move from target is valid.
            const auto TargetWorldPos = LevelInstance->GridToWorld(Target.Tile);
            const FVector TargetWorldVec{TargetWorldPos.X, TargetWorldPos.Y, 0.0f};
            const auto Dir = IntendedMoveDirection.IsNonZero() ? IntendedMoveDirection : CurrentMoveDirection;
            const auto Result = LevelInstance->ComputeTargetTile(World, TargetWorldVec, Dir, TagsToCollideWith, TEXT("Player"));

            // Update our actor's state depending on the result.
            Target = Result;
            if (Result.IsValid) CurrentMoveDirection = Dir;

            // Finally, update our actor's position depending on the result.
            if (Result.IsValid)
            {
                const FVector NewLocation{
                    TargetWorldPos.X + Dir.X * AmountMovedPast,
                    TargetWorldPos.Y + Dir.Y * AmountMovedPast,
                    0.0f};
                MovablePawn->SetActorLocation(NewLocation);
            }
            else
            {
                const FVector NewLocation{TargetWorldPos.X, TargetWorldPos.Y, 0.0f};
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
        Target = FComputeTargetTileResult::Invalid();
        CurrentMoveDirection.X = InitialMoveDirection.X;
        CurrentMoveDirection.Y = InitialMoveDirection.Y;
    }
}
