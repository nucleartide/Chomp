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

    // Attach event handlers.
    GetWorld()->GetGameState<AChompGameState>()->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleGamePlayingStateChanged);
    GetWorld()->GetGameState<AChompGameState>()->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleGameStateChanged);

    if (!IsTestOriginAndDestinationEnabled)
    {
#if false
        // Then invoke our Scatter() behavior.
        // This could be made to a conditional (state could be Scatter or Chase),
        // but for now let's say that our config always has Scatter as the first mode.
        Scatter(StartingPosition, ScatterDestination);
#endif

        // Early return to avoid the logic below.
        return;
    }

    // Initialize pawn to configured origin.
    auto LevelInstance = ULevelLoader::GetInstance(Level);
    auto OriginWorldPosition = LevelInstance->GridToWorld(Origin);
    FVector OriginWorldPos(OriginWorldPosition.X, OriginWorldPosition.Y, 0.0f);
    GetPawn()->SetActorLocation(OriginWorldPos);

    // Call out to Pathfind().
    std::unordered_map<FGridLocation, FGridLocation> CameFrom;
    std::unordered_map<FGridLocation, double> CostSoFar;
    std::function<double(FGridLocation, FGridLocation)> FunctionObject = &AStar::ManhattanDistanceHeuristic;
    AStar::Pathfind<FGridLocation>(
        LevelInstance,
        Origin,
        Destination,
        CameFrom,
        CostSoFar,
        FunctionObject);

    // Debug.
    DebugAStar(CameFrom);

    // Reconstruct path, and save.
    auto Path = AStar::ReconstructPath(Origin, Destination, CameFrom);
    check(Path.size() >= 2);
    CurrentPath.Initialize(Path);

    // Initialize movement.
    auto Current = Path[0];
    auto Next = Path[1];
    StartMovingFrom(Current, Next);
}

void AGhostAIController::DebugAStar(std::unordered_map<FGridLocation, FGridLocation> &CameFrom)
{
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

void AGhostAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    auto CurrentGameState = GetWorld()->GetGameState<AChompGameState>()->GetEnum();
    if (CurrentGameState != EChompGameState::Playing)
        return;

    // Move the Ghost, given the currently saved Source and Destination.
    Move(DeltaTime);

    auto CurrentWave = GetWorld()->GetGameState<AChompGameState>()->GetCurrentWave();
    if (IsAtDestination && CurrentWave == EChompGamePlayingState::Scatter)
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
            // We've arrived. Swap the ScatterOrigin and ScatterDestination first.
            FGridLocation Temp{ScatterOrigin.X, ScatterOrigin.Y};
            ScatterOrigin = ScatterDestination;
            ScatterDestination = Temp;

            // Then invoke Scatter() once again.
            Scatter(ScatterOrigin, ScatterDestination);
        }
    }
    else if (IsAtDestination && CurrentWave == EChompGamePlayingState::Chase)
    {
        // Re-evaluate A* after every grid-node visit.
        Chase();
    }
}

void AGhostAIController::HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState)
{
    // Preconditions.
    check(OldState != NewState);

    // TODO: This will need to be changed once ghosts can be consumed, but for now this works.
    if (NewState == EChompGameState::Playing)
    {
        // Set the starting position.
        auto LevelInstance = ULevelLoader::GetInstance(Level);
        auto StartingPosition = GetPawn<AGhostPawn>()->GetStartingPosition();
        auto StartingWorldPosition = LevelInstance->GridToWorld(StartingPosition);
        FVector StartingWorldPos(StartingWorldPosition.X, StartingWorldPosition.Y, 0.0f);
        GetPawn()->SetActorLocation(StartingWorldPos);
    }
}

void AGhostAIController::HandleGamePlayingStateChanged(EChompGamePlayingState OldState, EChompGamePlayingState NewState)
{
    check(OldState != NewState);

    if (NewState == EChompGamePlayingState::Scatter)
    {
        // Set the starting position.
        auto LevelInstance = ULevelLoader::GetInstance(Level);
        auto ActorLocation = GetPawn<AGhostPawn>()->GetActorLocation();
        FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
        auto ActorGridLocation = LevelInstance->WorldToGrid(ActorLocation2D);

        // Then invoke our Scatter() behavior.
        Scatter(ActorGridLocation, ScatterDestination);
    }
    else if (NewState == EChompGamePlayingState::Chase)
    {
        // Then invoke our Chase() behavior.
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

void AGhostAIController::Move(float DeltaTime)
{
    if (IsAtDestination)
        return;

    // Compute the movement direction.
    FVector2D MovementDirection(CurrentDestinationGridPos.X - CurrentOriginGridPos.X, CurrentDestinationGridPos.Y - CurrentOriginGridPos.Y);

    // Magnify the movement direction by the movement speed and delta time.
    auto ScaledMovementDirection = MovementDirection.GetSafeNormal() * DeltaTime * MovementSpeed;

    // Grab a reference to the ghost pawn.
    auto GhostPawn = Cast<AGhostPawn>(GetPawn());
    check(GhostPawn);

    // Move the ghost pawn.
    GhostPawn->MoveVector(ScaledMovementDirection, DeltaTime);

    // Pawn has exceeded destination if...
    auto ActorLocation = GetPawn()->GetActorLocation();
    bool ExceededDestination = false;
    auto Dest = ULevelLoader::GetInstance(Level)->GridToWorld(CurrentDestinationGridPos);
    if (MovementDirection.Y < 0)
        ExceededDestination = ActorLocation.Y <= Dest.Y;
    else if (MovementDirection.Y > 0)
        ExceededDestination = ActorLocation.Y >= Dest.Y;
    else if (MovementDirection.X < 0)
        ExceededDestination = ActorLocation.X <= Dest.X;
    else if (MovementDirection.X > 0)
        ExceededDestination = ActorLocation.X >= Dest.X;

    // If the pawn has exceeded the destination, update internal bookkeeping.
    if (ExceededDestination)
    {
        IsAtDestination = true;
    }
}

void AGhostAIController::Scatter(FGridLocation _ScatterOrigin, FGridLocation _ScatterDestination)
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
    auto Path = AStar::ReconstructPath(ActorGridLocation, _ScatterDestination, CameFrom);
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
    // DebugAStar(CameFrom);

    // Reconstruct and save the path.
    auto Path = AStar::ReconstructPath(ActorGridLocation, PlayerGridLocation, CameFrom);
    check(Path.size() >= 2);
    CurrentPath.Initialize(Path);

    // Finally, initialize moving on the path.
    auto Current = Path[0];
    auto Next = Path[1];
    StartMovingFrom(Current, Next);
}
