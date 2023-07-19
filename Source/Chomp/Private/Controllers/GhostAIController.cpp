#include "Controllers/GhostAIController.h"

#include "Math/UnrealMathUtility.h"
#include "VectorTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "LevelGenerator/LevelLoader.h"
#include "AStar/AStar.h"
#include "ChompGameState.h"

void AGhostAIController::BeginPlay()
{
    Super::BeginPlay();
    auto ChompGameState = GetWorld()->GetGameState<AChompGameState>();
    ChompGameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleGamePlayingStateChanged);
    ChompGameState->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleGameStateChanged);
    ChompGameState->OnDotsConsumedUpdatedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleDotsConsumedChanged);
}

void AGhostAIController::DebugAStar(std::unordered_map<FGridLocation, FGridLocation> &CameFrom)
{
    if (!Debug)
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
                auto From = CameFrom[FGridLocation{X, Y}];
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

void AGhostAIController::MoveTowardDestination(float DeltaTime)
{
    if (IsAtDestination)
        return;

    // Compute the movement direction.
    FGridLocation MovementDirection{CurrentDestinationGridPos.X - CurrentOriginGridPos.X, CurrentDestinationGridPos.Y - CurrentOriginGridPos.Y};

    // Move the ghost pawn.
    GetPawn<AGhostPawn>()->MoveTowards(MovementDirection, DeltaTime);

    // Grab some values we'll need below.
    auto ActorLocation = GetPawn()->GetActorLocation();
    auto DestLocation = ULevelLoader::GetInstance(Level)->GridToWorld(CurrentDestinationGridPos);

    // Determine whether pawn has reached its destination.
    IsAtDestination = false;
    if (MovementDirection.Y < 0)
        IsAtDestination = ActorLocation.Y <= DestLocation.Y;
    else if (MovementDirection.Y > 0)
        IsAtDestination = ActorLocation.Y >= DestLocation.Y;
    else if (MovementDirection.X < 0)
        IsAtDestination = ActorLocation.X <= DestLocation.X;
    else if (MovementDirection.X > 0)
        IsAtDestination = ActorLocation.X >= DestLocation.X;
}

void AGhostAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Early return if not playing.
    auto CurrentGameState = GetWorld()->GetGameState<AChompGameState>()->GetEnum();
    if (CurrentGameState != EChompGameState::Playing)
        return;
    if (!DidStartMoving)
        return;

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

void AGhostAIController::HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState)
{
    check(OldState != NewState);
    if (NewState == EChompGameState::Playing)
    {
        // Set the starting position of the pawn.
        auto StartingGridPosition = GetPawn<AGhostPawn>()->GetStartingPosition();
        auto StartingWorldPosition = ULevelLoader::GetInstance(Level)->GridToWorld(StartingGridPosition);
        FVector StartingWorldPos(StartingWorldPosition.X, StartingWorldPosition.Y, 0.0f);
        GetPawn()->SetActorLocation(StartingWorldPos);
    }
}

void AGhostAIController::HandleGamePlayingStateChanged(EChompGamePlayingState OldState, EChompGamePlayingState NewState)
{
    check(OldState != NewState);
    if (NewState == EChompGamePlayingState::Scatter)
        Scatter();
    else if (NewState == EChompGamePlayingState::Chase)
        Chase();
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

void AGhostAIController::HandleDotsConsumedChanged(int NumberOfDotsConsumed)
{
    DEBUG_LOG(TEXT("new dots consumed: %d"), NumberOfDotsConsumed);

    auto GhostPawn = GetPawn<AGhostPawn>();
    check(GhostPawn);

    auto Threshold = GhostPawn->GetDotsConsumedMovementThreshold();
    if (NumberOfDotsConsumed >= Threshold)
    {
        DidStartMoving = true;
    }
}
