#include "Controllers/GhostAIController.h"
#include "Math/UnrealMathUtility.h"
#include "AStar/AStar.h"
#include "ChompGameState.h"
#include "Kismet/GameplayStatics.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"

void AGhostAIController::BeginPlay()
{
	Super::BeginPlay();

	auto World = GetWorld();
	check(World);

	auto ChompGameState = World->GetGameState<AChompGameState>();
	ChompGameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(
		this, &AGhostAIController::HandleGamePlayingSubstateChanged);
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

	// Old code reference for when a target has been exceeded.
#if false
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
    
            // Then invoke ComputePath() once again.
            ComputePath();
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
            // We've arrived. Invoke ComputeChasePath() once again.
            ComputeChasePath();
        }
    }
#endif
}

/**
 * Sync the GhostAIController with the playing sub-state of the game.
 */
// ReSharper disable once CppMemberFunctionMayBeStatic
void AGhostAIController::HandleGamePlayingSubstateChanged(EChompGamePlayingSubstate OldState,
                                                          EChompGamePlayingSubstate NewState)
{
	check(OldState != NewState);
	if (NewState == EChompGamePlayingSubstate::Scatter)
		ComputeScatterPath();
	else if (NewState == EChompGamePlayingSubstate::Chase)
		ComputeChasePath();
}

/**
 * When the game starts playing, reset the position of the pawn.
 */
// ReSharper disable once CppMemberFunctionMayBeConst
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

std::vector<FGridLocation> AGhostAIController::ComputePath(
	ULevelLoader* LevelInstance,
	FVector2D CurrentWorldPosition,
	FGridLocation OriginGridPosition,
	FGridLocation DestinationGridPosition,
	bool DebugAStarMap = true)
{
	// Compute the A* path anew.
	std::unordered_map<FGridLocation, FGridLocation> CameFrom;
	std::unordered_map<FGridLocation, double> CostSoFar;
	std::function FunctionObject = &AStar::ManhattanDistanceHeuristic;
	AStar::Pathfind<FGridLocation>(
		LevelInstance,
		OriginGridPosition,
		DestinationGridPosition,
		CameFrom,
		CostSoFar,
		FunctionObject);

	// Debug the results of running A*.
	if (DebugAStarMap)
		DebugAStar(CameFrom, LevelInstance);

	// Reconstruct and return the computed path.
	auto Path = AStar::ReconstructPath(
		CurrentWorldPosition,
		OriginGridPosition,
		DestinationGridPosition,
		CameFrom);
	check(Path.size() >= 2);
	return Path;
}

bool AGhostAIController::CanStartMoving() const
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

	return Threshold >= 0 && NumberOfDotsConsumed >= 0 && NumberOfDotsConsumed >= Threshold;
}

void AGhostAIController::DebugAStar(const std::unordered_map<FGridLocation, FGridLocation>& CameFrom,
                                    ULevelLoader* LevelInstance)
{
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

void AGhostAIController::ComputeScatterPath()
{
	auto Pawn = GetPawn<AMovablePawn>();
	check(Pawn);
	auto WorldLocation = Pawn->GetActorLocation2D();
	auto GridLocation = Pawn->GetGridLocation();

	auto Path = ComputePath(ULevelLoader::GetInstance(Level), WorldLocation, GridLocation, ScatterDestination,
	                        DebugAStarMap);
	MovementPath.Initialize(Path);
}

void AGhostAIController::ComputeChasePath()
{
	auto Pawn = GetPawn<AMovablePawn>();
	check(Pawn);
	auto WorldLocation = Pawn->GetActorLocation2D();
	auto GridLocation = Pawn->GetGridLocation();

	auto World = GetWorld();
	check(World);
	auto PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	check(PlayerController);
	auto PlayerPawn = PlayerController->GetPawn<AMovablePawn>();
	check(PlayerPawn);
	auto PlayerGridLocation = PlayerPawn->GetGridLocation();

	auto Path = ComputePath(ULevelLoader::GetInstance(Level), WorldLocation, GridLocation, PlayerGridLocation,
	                        DebugAStarMap);
	MovementPath.Initialize(Path);
}
