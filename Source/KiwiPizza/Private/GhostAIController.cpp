#include "GhostAIController.h"
#include "GhostPawn.h"
#include "Math/UnrealMathUtility.h"
#include "VectorTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Debug.h"
#include "LevelLoader.h"

void AGhostAIController::BeginPlay()
{
    Super::BeginPlay();

    if (IsTestOriginAndDestinationEnabled)
    {
        // Get current world position.
        auto ActorLocation = GetPawn()->GetActorLocation();
        FVector2D ActorLocation2D(ActorLocation.X, ActorLocation.Y);

        // Convert the world position to grid position. This will be the origin.
        auto Origin = ULevelLoader::GetInstance(Level)->WorldToGrid(ActorLocation2D);

        // Add 10 to the x component of the grid position. This will be the destination.
        GridPosition Destination(Origin.X + 10, Origin.Y);

        // Start moving from the origin to the destination specified above.
        StartMovingFrom(Origin, Destination);
    }
}

void AGhostAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Move the Ghost, given the currently saved Source and Destination.
    Move(DeltaTime);
}

void AGhostAIController::StartMovingFrom(GridPosition Origin, GridPosition Destination)
{
    // Ensure Origin and Destination are axis-aligned.
    check((Origin.X == Destination.X) != (Origin.Y == Destination.Y));

    // Save the current Origin and Destination.
    CurrentOriginGridPos = Origin;
    CurrentDestinationGridPos = Destination;

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

void AGhostAIController::Pathfind(GridPosition Destination)
{
    // TODO.
    // Need to customize yesterday's A-star implementation.
    // Just use the Manhattan distance heuristic for now, we can rationalize the heuristic later.

    // TODO: Construct a Graph. Let's do this after dinner.
    // ...

    /*
template <typename Location, typename Graph>
void a_star_search(Graph graph,
                   Location start,
                   Location goal,
                   std::unordered_map<Location, Location> &came_from,
                   std::unordered_map<Location, double> &cost_so_far)
    */
}
