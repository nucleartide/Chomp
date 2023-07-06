#include "GhostAIController.h"
#include "GhostPawn.h"
#include "Math/UnrealMathUtility.h"
#include "VectorTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Debug.h"
#include "LevelLoader.h"
#include "AStar.h"

/*
// This outputs a grid. Pass in a distances map if you want to print
// the distances, or pass in a point_to map if you want to print
// arrows that point to the parent location, or pass in a path vector
// if you want to draw the path.
void draw_grid(IGraph *graph,
               std::unordered_map<FGridLocation, double> *distances = nullptr,
               std::unordered_map<FGridLocation, FGridLocation> *point_to = nullptr,
               std::vector<FGridLocation> *path = nullptr,
               FGridLocation *start = nullptr,
               FGridLocation *goal = nullptr)
{
    const int field_width = 3;
    std::cout << std::string(field_width * graph.width, '_') << '\n';
    for (int y = 0; y != graph.height; ++y)
    {
        for (int x = 0; x != graph.width; ++x)
        {
            FGridLocation id{x, y};
            if (graph.walls.find(id) != graph.walls.end())
            {
                std::cout << std::string(field_width, '#');
            }
            else if (start && id == *start)
            {
                std::cout << " A ";
            }
            else if (goal && id == *goal)
            {
                std::cout << " Z ";
            }
            else if (path != nullptr && find(path->begin(), path->end(), id) != path->end())
            {
                std::cout << " @ ";
            }
            else if (point_to != nullptr && point_to->count(id))
            {
                FGridLocation next = (*point_to)[id];
                if (next.x == x + 1)
                {
                    std::cout << " > ";
                }
                else if (next.x == x - 1)
                {
                    std::cout << " < ";
                }
                else if (next.y == y + 1)
                {
                    std::cout << " v ";
                }
                else if (next.y == y - 1)
                {
                    std::cout << " ^ ";
                }
                else
                {
                    std::cout << " * ";
                }
            }
            else if (distances != nullptr && distances->count(id))
            {
                std::cout << ' ' << std::left << std::setw(field_width - 1) << (*distances)[id];
            }
            else
            {
                std::cout << " . ";
            }
        }
        std::cout << '\n';
    }
    std::cout << std::string(field_width * graph.width, '~') << '\n';
}
*/

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
        // auto Origin = ULevelLoader::GetInstance(Level)->WorldToGrid(ActorLocation2D);

        // Add 10 to the x component of the grid position. This will be the destination.
        // FGridLocation Destination{Origin.X + 10, Origin.Y};

        // Start moving from the origin to the destination specified above.
        StartMovingFrom(Origin, Destination);

        // Call out to AStarSearch().
        std::unordered_map<FGridLocation, FGridLocation> CameFrom;
        std::unordered_map<FGridLocation, double> CostSoFar;
        std::function<double(FGridLocation, FGridLocation)> FunctionObject = &ManhattanDistanceHeuristic;
        AStarSearch(
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
                auto To = FGridLocation{X, Y};
                auto From = CameFrom[FGridLocation{X, Y}];
                if (!LevelInstance->Passable(From, To))
                {
                    Line += TEXT("X");
                }
                else
                {
                    auto Parent = CameFrom[FGridLocation{X, Y}];
                    if (Parent.X < X)
                        Line += TEXT("v");
                    else if (Parent.X > X)
                        Line += TEXT("^");
                    else if (Parent.Y < Y)
                        Line += TEXT("<");
                    else if (Parent.Y > Y)
                        Line += TEXT(">");
                    else
                    {
                        Line += TEXT(" ");
                    }
                }
            }
            DEBUG_LOG(TEXT("%s"), *Line);
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
