#include "Controllers/GhostAIController.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "VectorTypes.h"

#include "AStar/AStar.h"
#include "ChompGameState.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"

void AGhostAIController::BeginPlay()
{
    Super::BeginPlay();

    auto World = GetWorld();
    check(World);

    auto ChompGameState = World->GetGameState<AChompGameState>();
    ChompGameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleGamePlayingStateChanged);
    ChompGameState->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleGameStateChanged);
}

void AGhostAIController::Tick(float DeltaTime)
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

    // Early return if ghost can't move yet.
    if (!CanStartMoving())
        return;

    UpdateIntendedMoveDirection();

#if false
    // Move the pawn.
    MoveTowardDestination(DeltaTime);
    if (!IsAtDestination)
        return;

    // Once we are at our destination, do different things depending on the current "Game Playing" substate.
    auto PlayingSubstate = GetWorld()->GetGameState<AChompGameState>()->GetPlayingSubstate();
    if (PlayingSubstate == EChompGamePlayingState::Scatter)
        HandleScatterNodeReached();
    else if (PlayingSubstate == EChompGamePlayingState::Chase)
        HandleChaseNodeReached();
#endif

#if false
    // Pseudocode:
    //
    // Process input and update intended direction (path).
    // If the target tile hasn't been set,
    //     Attempt to set the target tile using the current direction
    //     If the attempt fails (and it will on first tick b/c the current direction is zero), set the current direction from the path
    // If the target tile has been set,
    //     Move toward the target tile
    // .   If we've moved past the target tile,
    // .       Update the path
    // .       update the actor's position along this path
    // .       Unset the target tile. We'll compute this again on next loop

    // Process input.
    UpdateIntendedMoveDirection();

    // Some values we need for later.
    auto LevelInstance = ULevelLoader::GetInstance(Level);
    auto ActorLocation = MovablePawn->GetActorLocation();
    auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();

    // If target tile hasn't been set, attempt to set the target tile.
    if (!Target.IsValid)
    {
        Target = LevelInstance->ComputeTargetTile(World, ActorLocation, CurrentMoveDirection, TagsToCollideWith);
    }

    // If intended move direction is non-zero and different, also attempt to set the target tile.
    if (Target.IsValid && IntendedMoveDirection.IsNonZero() && CurrentMoveDirection != IntendedMoveDirection)
    {
        auto Result = LevelInstance->ComputeTargetTile(World, ActorLocation, IntendedMoveDirection, TagsToCollideWith);
        if (Result.IsValid)
        {
            Target.Tile = Result.Tile;
            CurrentMoveDirection = IntendedMoveDirection;
        }
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
            // TODO: this seems buggy. why not just set the CurrentMoveDirection first?
            // TODO: simplify the logic in ChompPlayerController and test to ensure it works.

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
#endif
}

void AGhostAIController::UpdateMovementPath(Path &MovementPath)
{
    // TODO.
}

/**
 * Sync the GhostAIController with the playing substate of the game.
 */
void AGhostAIController::HandleGamePlayingStateChanged(EChompGamePlayingState OldState, EChompGamePlayingState NewState)
{
    check(OldState != NewState);
    if (NewState == EChompGamePlayingState::Scatter)
        Scatter();
    else if (NewState == EChompGamePlayingState::Chase)
        Chase();
}

/**
 * When the game starts playing, reset the position of the pawn.
 */
void AGhostAIController::HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState)
{
    check(OldState != NewState);
    if (NewState == EChompGameState::Playing)
    {
        // Set the starting position of the pawn.
        auto GhostPawn = GetPawn<AGhostPawn>();
        check(GhostPawn);
        auto StartingGridPosition = GhostPawn->GetStartingPosition();
        auto StartingWorldPosition = ULevelLoader::GetInstance(Level)->GridToWorld(StartingGridPosition);
        FVector StartingWorldPos(StartingWorldPosition.X, StartingWorldPosition.Y, 0.0f);
        GetPawn()->SetActorLocation(StartingWorldPos);
    }
}

void AGhostAIController::Scatter()
{
}

void AGhostAIController::Chase()
{
}

bool AGhostAIController::CanStartMoving()
{
    int Threshold = -1;
    {
        auto GhostPawn = GetPawn<AGhostPawn>();
        check(GhostPawn);
        Threshold = GhostPawn->GetDotsConsumedMovementThreshold();
    }

    int NumberOfDotsConsumed = -1;
    {
        auto World = GetWorld();
        check(World);
        auto ChompGameState = World->GetGameState<AChompGameState>();
        NumberOfDotsConsumed = ChompGameState->GetNumberOfDotsConsumed();
    }

    return Threshold >= 0 && NumberOfDotsConsumed >=0 && NumberOfDotsConsumed >= Threshold;
}

void AGhostAIController::DebugAStar(const std::unordered_map<FGridLocation, FGridLocation> &CameFrom)
{
    if (!DebugAStarMap)
        return;

    auto LevelInstance = ULevelLoader::GetInstance(Level);
    for (int X = LevelInstance->GetLevelHeight() - 1; X >= 0; X--)
    {
        FString Line = TEXT("");
        for (int Y = 0; Y < LevelInstance->GetLevelWidth(); Y++)
        {
            if (CameFrom.find(FGridLocation{X, Y}) == CameFrom.end())
            {
                Line += TEXT("W");
            }
            else
            {
                auto From = CameFrom.at(FGridLocation{X, Y});
                auto To = FGridLocation{X, Y};
                if (From.X < X)
                    Line += TEXT("v");
                else if (From.X > X)
                    Line += TEXT("^");
                else if (From.Y < Y)
                    Line += TEXT("<");
                else if (From.Y > Y)
                    Line += TEXT(">");
                else
                    Line += TEXT("@");
            }
        }
        DEBUG_LOG(TEXT("%s"), *Line);
    }
}

#if false
void AGhostAIController::MoveTowardDestination(float DeltaTime)
{
    if (IsAtDestination)
        return;

    // Compute the movement direction.
    FGridLocation MovementDirection{CurrentDestinationGridPos.X - CurrentOriginGridPos.X, CurrentDestinationGridPos.Y - CurrentOriginGridPos.Y};

    // Move the ghost pawn.
    // GetPawn<AGhostPawn>()->MoveTowards(MovementDirection, DeltaTime);
}

void AGhostAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AGhostAIController::HandleScatterNodeReached()
{
    // Then bump our index.
    CurrentPath.CurrentIndex++;

    // If CurrentIndex is not at the last index,
    if (CurrentPath.CurrentIndex < CurrentPath.Locations.size() - 1)
    {
        // Start moving on the next path segment.
        auto Current = CurrentPath.Locations[CurrentPath.CurrentIndex];
        auto Next = CurrentPath.Locations[CurrentPath.CurrentIndex + 1];
        StartMovingFrom(Current, Next);
    }
    else // Otherwise,
    {
        // We've arrived. Swap the ScatterOrigin and ScatterDestination.
        FGridLocation Temp{ScatterOrigin.X, ScatterOrigin.Y};
        ScatterOrigin = ScatterDestination;
        ScatterDestination = Temp;

        // Then invoke Scatter() once again.
        Scatter();
    }
}

void AGhostAIController::HandleChaseNodeReached()
{
    // Then bump our index.
    CurrentPath.CurrentIndex++;

    // If CurrentIndex is not at the second index,
    if (CurrentPath.CurrentIndex < 1)
    {
        // Start moving on the next path segment.
        auto Current = CurrentPath.Locations[CurrentPath.CurrentIndex];
        auto Next = CurrentPath.Locations[CurrentPath.CurrentIndex + 1];
        StartMovingFrom(Current, Next);
    }
    else // Otherwise,
    {
        // We've arrived. Invoke Chase() once again.
        Chase();
    }
}

void AGhostAIController::StartMovingFrom(FGridLocation _Origin, FGridLocation _Destination)
{
    // Save the current Origin and Destination.
    CurrentOriginGridPos = _Origin;
    CurrentDestinationGridPos = _Destination;

    // Reset some internal bookkeeping.
    IsAtDestination = false;
}

void AGhostAIController::Scatter()
{
    // Compute the current grid position of the pawn.
    auto ActorLocation = GetPawn()->GetActorLocation();
    FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
    auto LevelInstance = ULevelLoader::GetInstance(Level);
    auto ActorGridLocation = LevelInstance->WorldToGrid(ActorLocation2D);

    // Given the current grid position of the pawn,
    // as well as the ScatterDestination of the pawn,
    // call out to AStar::Pathfind().
    std::unordered_map<FGridLocation, FGridLocation> CameFrom;
    std::unordered_map<FGridLocation, double> CostSoFar;
    std::function<double(FGridLocation, FGridLocation)> FunctionObject = &AStar::ManhattanDistanceHeuristic;
    AStar::Pathfind<FGridLocation>(
        LevelInstance,
        ActorGridLocation,
        ScatterDestination,
        CameFrom,
        CostSoFar,
        FunctionObject);

    // Debug the results of running A*.
    DebugAStar(CameFrom);

    // Reconstruct and save the path.
    auto Path = AStar::ReconstructPath(ActorLocation2D, ActorGridLocation, ScatterDestination, CameFrom);
    check(Path.size() >= 2);
    CurrentPath.Initialize(Path);

    // Finally, initialize moving on the path.
    auto Current = Path[0];
    auto Next = Path[1];
    StartMovingFrom(Current, Next);
}

void AGhostAIController::Chase()
{
    // Grab reference to level instance.
    auto LevelInstance = ULevelLoader::GetInstance(Level);

    // Compute the current grid position of the pawn.
    FGridLocation ActorGridLocation;
    {
        auto ActorLocation = GetPawn()->GetActorLocation();
        FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
        ActorGridLocation = LevelInstance->WorldToGrid(ActorLocation2D);
    }

    // Compute the current grid position of the player.
    FGridLocation PlayerGridLocation;
    {
        auto PlayerLocation = UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetPawn()->GetActorLocation();
        FVector2D ActorLocation2D{PlayerLocation.X, PlayerLocation.Y};
        PlayerGridLocation = LevelInstance->WorldToGrid(ActorLocation2D);
    }

    // Given the two grid positions above,
    // call out to AStar::Pathfind().
    std::unordered_map<FGridLocation, FGridLocation> CameFrom;
    std::unordered_map<FGridLocation, double> CostSoFar;
    std::function<double(FGridLocation, FGridLocation)> FunctionObject = &AStar::ManhattanDistanceHeuristic;
    AStar::Pathfind<FGridLocation>(
        LevelInstance,
        ActorGridLocation,
        PlayerGridLocation,
        CameFrom,
        CostSoFar,
        FunctionObject);

    // Debug the results of running A*.
    DebugAStar(CameFrom);

    // Reconstruct and save the path.
    auto ActorLocation = GetPawn()->GetActorLocation();
    FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
    auto Path = AStar::ReconstructPath(ActorLocation2D, ActorGridLocation, PlayerGridLocation, CameFrom);
    check(Path.size() >= 2);
    CurrentPath.Initialize(Path);

    // Finally, initialize moving on the path.
    auto Current = Path[0];
    auto Next = Path[1];
    StartMovingFrom(Current, Next);
}
#endif
