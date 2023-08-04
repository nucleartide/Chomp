#include "Controllers/GhostAiController.h"

#include "AGhostHouseQueue.h"
#include "Math/UnrealMathUtility.h"
#include "AStar/AStar.h"
#include "ChompGameState.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"
#include "Pawns/MovablePawn.h"
#include "Pawns/Movement/MovementResult.h"

void AGhostAiController::BeginPlay()
{
	Super::BeginPlay();

	// Add this instance of AGhostAiController to our RuntimeSet.
	if (IsStartingPositionInGhostHouse())
		GetGhostHouseQueue()->AddAndSort(this);

	// Initialize CurrentScatterOrigin and CurrentScatterDestination from Pawn.
	{
		const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
		CurrentScatterOrigin = Pawn->GetScatterOrigin();
		CurrentScatterDestination = Pawn->GetScatterDestination();
	}

	// Attach some handlers for when game state changes.
	{
		const auto GameState = FSafeGet::GameState<AChompGameState>(this);
		GameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(
			this,
			&AGhostAiController::HandleGamePlayingSubstateChanged);
	}
}

void AGhostAiController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Should have been initialized in BeginPlay().
	if (!MovementPath.IsValid())
		return;

	auto World = GetWorld();
	check(World);

	auto GameState = World->GetGameState<AChompGameState>();
	check(GameState);

	if (GameState->GetEnum() != EChompGameState::Playing)
		return;

	if (!CanStartMoving())
		return;

	// Compute new location and rotation.
	const auto MovablePawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto [NewLocation, NewRotation] = MovablePawn->MoveAlongPath(
		MovementPath,
		DeltaTime
	);

	// Apply new location.
	MovablePawn->SetActorLocationAndRotation(NewLocation, NewRotation);

	// Compute a new movement path if conditions are met.
	if (auto PlayingSubstate = GameState->GetPlayingSubstate();
		PlayingSubstate == EChompGamePlayingSubstate::Scatter &&
		MovementPath.WasCompleted(NewLocation))
	{
		auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
		auto Destination = Pawn->GetScatterDestination();
		ComputeScatterForMovementPath(Destination);
		SwapScatterOriginAndDestination();
	}
	else if (PlayingSubstate == EChompGamePlayingSubstate::Chase && MovementPath.DidComplete(NewLocation, 1))
	{
		ComputeChaseForMovementPath();
	}
}

/**
 * Sync the GhostAIController with the playing sub-state of the game.
 */
// ReSharper disable once CppMemberFunctionMayBeStatic
void AGhostAiController::HandleGamePlayingSubstateChanged(EChompGamePlayingSubstate OldState,
                                                          EChompGamePlayingSubstate NewState)
{
	check(OldState != NewState);
	if (NewState == EChompGamePlayingSubstate::Scatter)
	{
		auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
		auto Destination = Pawn->GetScatterDestination();
		ComputeScatterForMovementPath(Destination);
	}
	else if (NewState == EChompGamePlayingSubstate::Chase)
	{
		ComputeChaseForMovementPath();
	}
}

/**
 * When the game starts playing, reset the position of the pawn.
 */
void AGhostAiController::HandleGameStateChanged(EChompGameState OldState, EChompGameState NewState)
{
	check(OldState != NewState);
	if (NewState == EChompGameState::Playing)
	{
		ResetPawnPosition();
	}
}

TArray<FGridLocation> AGhostAiController::ComputePath(
	ULevelLoader* LevelInstance,
	FVector2D CurrentWorldPosition,
	FGridLocation StartGridPos,
	FGridLocation EndGridPos,
	bool Debug = true)
{
	// Compute A* path.
	std::unordered_map<FGridLocation, FGridLocation> CameFrom;
	std::unordered_map<FGridLocation, double> CostSoFar;
	const std::function FunctionObject = &FAStar::ManhattanDistanceHeuristic;
	FAStar::Pathfind<FGridLocation>(
		LevelInstance,
		StartGridPos,
		EndGridPos,
		CameFrom,
		CostSoFar,
		FunctionObject);

	// Debug if enabled.
	if (Debug)
		DebugAStar(CameFrom, LevelInstance);

	// Reconstruct the path.
	auto Path = FAStar::ReconstructPath(
		StartGridPos,
		EndGridPos,
		CameFrom);
	check(Path.Num() >= 1);
	check(Path[0] == StartGridPos);

	// Assert that ghost is axis-aligned.
	auto StartWorldPos = LevelInstance->GridToWorld(StartGridPos);
	check(StartWorldPos.X == CurrentWorldPosition.X || StartWorldPos.Y == CurrentWorldPosition.Y);

	// Return computed path.
	return Path;
}

bool AGhostAiController::CanStartMoving() const
{
	return true;

#if false
	// get threshold
	int Threshold = -1;
	{
		auto GhostPawn = GetPawn<AGhostPawn>();
		check(GhostPawn);
		Threshold = GhostPawn->GetDotsConsumedMovementThreshold();
	}

	// get number of dots consumed
	int NumberOfDotsConsumed = -1;
	{
		auto World = GetWorld();
		check(World);
		auto ChompGameState = World->GetGameState<AChompGameState>();
		NumberOfDotsConsumed = ChompGameState->GetNumberOfDotsConsumed();
	}

	// TODO
	// if the number of dots consumed has been exceeded,
	// remove from ghost house

	// TODO
	// in chompgamestate,
	// implement a "force remove" event if time since last dot consumed has been exceeded

	// TODO
	// if a "force remove event" has been received,
	// remove from ghost house

	// TODO
    // update the condition below: shouldn't be in ghost house queue, and movement path shouldn't have been completed.
	!MovementPath.WasCompleted(Pawn->GetActorLocation()) &&
	!IsInGhostHouse();
#endif
}

void AGhostAiController::DebugAStar(
	const std::unordered_map<FGridLocation, FGridLocation>& CameFrom,
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

void AGhostAiController::ComputeScatterForMovementPath(const FGridLocation& ScatterDestination)
{
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto WorldLocation = FVector2D(Pawn->GetActorLocation());
	const auto GridLocation = Pawn->GetGridLocation();
	const auto Path = ComputePath(ULevelLoader::GetInstance(Level), WorldLocation, GridLocation, ScatterDestination,
	                              DebugAStarMap);

	MovementPath = FMovementPath(Pawn->GetActorLocation(), Path, ULevelLoader::GetInstance(Level));
	check(MovementPath.IsValid());
	MovementPath.DebugLog(TEXT("Scatter"));
}

void AGhostAiController::ComputeChaseForMovementPath()
{
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto WorldLocation = FVector2D(Pawn->GetActorLocation());
	const auto GridLocation = Pawn->GetGridLocation();
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AMovablePawn>();
	if (!PlayerPawn)
		return;
	const auto PlayerWorldPosition = PlayerPawn->GetActorLocation();
	const auto PlayerGridLocation = PlayerPawn->GetGridLocation();
	const auto Path = ComputePath(ULevelLoader::GetInstance(Level), WorldLocation, GridLocation, PlayerGridLocation,
	                              DebugAStarMap);

	MovementPath = FMovementPath(Pawn->GetActorLocation(), Path, ULevelLoader::GetInstance(Level));
	check(MovementPath.IsValid());
	MovementPath.DebugLog(TEXT("Chase"));
}

void AGhostAiController::ResetPawnPosition() const
{
	// Set the starting position of the pawn.
	const auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto StartingGridPosition = GhostPawn->GetStartingPosition();
	const auto StartingWorldPosition = ULevelLoader::GetInstance(Level)->GridToWorld(StartingGridPosition);
	const FVector StartingWorldPos(StartingWorldPosition.X, StartingWorldPosition.Y, 0.0f);
	GetPawn()->SetActorLocation(StartingWorldPos);
}

void AGhostAiController::SwapScatterOriginAndDestination()
{
	const FGridLocation Swap{CurrentScatterOrigin.X, CurrentScatterOrigin.Y};
	CurrentScatterOrigin = CurrentScatterDestination;
	CurrentScatterDestination = Swap;
}

bool AGhostAiController::IsStartingPositionInGhostHouse() const
{
	// TODO: implement.
	return true;
}

bool AGhostAiController::IsInGhostHouse() const
{
	// TODO: implement
	return true;
}

int AGhostAiController::GetLeaveGhostHousePriority() const
{
	const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	return Pawn->GetLeaveGhostHousePriority();
}

AGhostHouseQueue* AGhostAiController::GetGhostHouseQueue() const
{
	const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	return Pawn->GetGhostHouseQueue();
}
