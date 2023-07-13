#include "Controllers/GhostAIController.h"

#include "Math/UnrealMathUtility.h"
#include "VectorTypes.h"
#include "Kismet/KismetMathLibrary.h"

#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "LevelGenerator/LevelLoader.h"
#include "AStar/AStar.h"

void AGhostAIController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsTestOriginAndDestinationEnabled)
        return;

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

    // Move the Ghost, given the currently saved Source and Destination.
    Move(DeltaTime);

    if (IsAtDestination)
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
            // We've arrived. Do nothing for the time being.
            // (No-op.)
        }
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
