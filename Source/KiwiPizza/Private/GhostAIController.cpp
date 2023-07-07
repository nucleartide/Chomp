#include "Math/UnrealMathUtility.h"
#include "VectorTypes.h"
#include "Kismet/KismetMathLibrary.h"

#include "GhostAIController.h"
#include "GhostPawn.h"
#include "Debug.h"
#include "LevelLoader.h"
#include "AStar.h"

void AGhostAIController::BeginPlay()
{
    Super::BeginPlay();

    if (IsTestOriginAndDestinationEnabled)
    {
        // Get current world position.
        auto ActorLocation = GetPawn()->GetActorLocation();
        FVector2D ActorLocation2D(ActorLocation.X, ActorLocation.Y);

        // Convert the world position to grid position. This will be the origin.
        auto LevelInstance = ULevelLoader::GetInstance(Level);

        // Start moving from the origin to the destination specified above.
        StartMovingFrom(Origin, Destination);

        // Call out to AStarSearch().
        std::unordered_map<FGridLocation, FGridLocation> CameFrom;
        std::unordered_map<FGridLocation, double> CostSoFar;
        std::function<double(FGridLocation, FGridLocation)> FunctionObject = &ManhattanDistanceHeuristic;
        AStarSearch<FGridLocation>(
            LevelInstance,
            Origin,
            Destination,
            CameFrom,
            CostSoFar,
            FunctionObject);
        DEBUG_LOG(TEXT("A-star search is complete. Check the breakpoint plz"));

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

        auto Path = ReconstructPath(Origin, Destination, CameFrom);
        DEBUG_LOG(TEXT("Vector below:"));
        for (auto Element : Path)
        {
            DEBUG_LOG(TEXT("%s"), *Element.ToString());
        }
    }
}

void AGhostAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Move the Ghost, given the currently saved Source and Destination.
    Move(DeltaTime);
}

void AGhostAIController::StartMovingFrom(FGridLocation _Origin, FGridLocation _Destination)
{
    // Ensure Origin and Destination are axis-aligned.
    // check((_Origin.X == _Destination.X) != (_Origin.Y == _Destination.Y));

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
    auto ActorLocation = GhostPawn->GetActorLocation();
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

// TODO: Be able to say `.Pathfind(FGridLocation{x, y})`, and have the ghost move to that location.
// [ ] Just use the Manhattan distance heuristic for now, we can rationalize the heuristic later.
void AGhostAIController::Pathfind(FGridLocation _Destination)
{
    /*
template <typename Location, typename Graph>
void a_star_search(Graph graph, - pass in the LevelLoader*
                   Location start, - this is FGridLocation
                   Location goal, - this is FGridLocation
                   const std::unordered_map<Location, Location> &came_from, -- this can be an empty map
                   const std::unordered_map<Location, double> &cost_so_far) -- this can be an empty map
    */
}
