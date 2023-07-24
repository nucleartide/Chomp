#include "Controllers/GhostAIController.h"
#include "Math/UnrealMathUtility.h"
#include "AStar/AStar.h"
#include "ChompGameState.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"

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

	auto World = GetWorld();
	check(World);

	auto GameState = World->GetGameState<AChompGameState>();
	check(GameState);

	if (GameState->GetEnum() != EChompGameState::Playing)
		return;

	if (!CanStartMoving())
		return;

	// Grab references to things.
	auto PlayingSubstate = GameState->GetPlayingSubstate();
	auto LevelInstance = ULevelLoader::GetInstance(Level);
	auto MovablePawn = FSafeGet::Pawn<AMovablePawn>(this);
	auto ActorLocation = MovablePawn->GetActorLocation();
	FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
	auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();
	auto GridLocation = LevelInstance->WorldToGrid(ActorLocation2D);

	//
	// In the code below, you should reason about the code while keeping 2 pieces of state in mind:
	// MovementPath, and Target
	//
	
	auto CurrentMoveDirection = MovementPath.GetCurrentMoveDirection(ActorLocation, LevelInstance);
	if (CurrentMoveDirection.IsZero())
	{
		MovementPath.Increment();
		return;
	}

	// Compute a new target tile if needed.
	if (!MovementPath.WasCompleted() && !Target.IsValid)
	{
		// Note that updates to our MovementPath are handled in GhostAIController callbacks.
		Target = LevelInstance->ComputeTargetTile(World, ActorLocation, CurrentMoveDirection, TagsToCollideWith,
		                                          TEXT("AI"));
	}

	// If there is no target tile, early return.
	if (!Target.IsValid)
		return;

	// Else, move toward the target.
	if (const auto [MovedPastTarget, AmountMovedPast] = MovablePawn->MoveTowardsPoint(
		Target.Tile, CurrentMoveDirection, DeltaTime); MovedPastTarget)
	{
		// Fetch the target world position before clearing the target below.
		const auto TargetWorldPos = LevelInstance->GridToWorld(Target.Tile);

		// Move onto the next node on the path, and invalidate the current target.
		// We'll compute a new target on the next loop iteration.
		MovementPath.Increment();
		Target = FComputeTargetTileResult::Invalid();

		// Align the position to the target.
		const FVector NewLocation{TargetWorldPos.X, TargetWorldPos.Y, 0.0f};
		MovablePawn->SetActorLocation(NewLocation);
	}

#if false

			// If there is a next point,
			const auto HasNextLocation = !MovementPath.WasCompleted();
			if (HasNextLocation)
			{
				// Compute the move direction.
				const auto [DirX, DirY] = MovementPath.GetCurrentMoveDirection();

				// Align the pawn to the target position + any extra amount in the new move direction.
				const FVector NewLocation{
					TargetWorldPos.X, // + DirX * AmountMovedPast,
					TargetWorldPos.Y, // + DirY * AmountMovedPast,
					0.0f
				};
				MovablePawn->SetActorLocation(NewLocation);
			}

			// Recompute the movement path if needed.
			if (PlayingSubstate == EChompGamePlayingSubstate::Scatter && !HasNextLocation)
			{
				FGridLocation Swap{ScatterOrigin.X, ScatterOrigin.Y};
				ScatterOrigin = ScatterDestination;
				ScatterDestination = Swap;
				ComputeScatterPath();
			}
			else if (PlayingSubstate == EChompGamePlayingSubstate::Chase && MovementPath.WasCompleted(0))
			{
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
		auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
		auto StartingGridPosition = GhostPawn->GetStartingPosition();
		auto StartingWorldPosition = ULevelLoader::GetInstance(Level)->GridToWorld(StartingGridPosition);
		FVector StartingWorldPos(StartingWorldPosition.X, StartingWorldPosition.Y, 0.0f);
		GetPawn()->SetActorLocation(StartingWorldPos);
	}
}

std::vector<FGridLocation> AGhostAIController::ComputePath(
	ULevelLoader* LevelInstance,
	FVector2D CurrentWorldPosition,
	FGridLocation StartGridPos,
	FGridLocation EndGridPos,
	bool Debug = true)
{
	// Compute A* path.
	std::unordered_map<FGridLocation, FGridLocation> CameFrom;
	std::unordered_map<FGridLocation, double> CostSoFar;
	const std::function FunctionObject = &AStar::ManhattanDistanceHeuristic;
	AStar::Pathfind<FGridLocation>(
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
	auto Path = AStar::ReconstructPath(
		StartGridPos,
		EndGridPos,
		CameFrom);
	check(Path[0] == StartGridPos);
	check(Path.size() >= 2);

	// Assert that ghost is axis-aligned.
	auto StartWorldPos = LevelInstance->GridToWorld(StartGridPos);
	check(StartWorldPos.X == CurrentWorldPosition.X || StartWorldPos.Y == CurrentWorldPosition.Y);

	// Return computed path.
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

	return Threshold >= 0 && NumberOfDotsConsumed >= 0 && NumberOfDotsConsumed >= Threshold && !MovementPath.
		WasCompleted();
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
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto WorldLocation = Pawn->GetActorLocation2D();
	const auto GridLocation = Pawn->GetGridLocation();
	const auto Path = ComputePath(ULevelLoader::GetInstance(Level), WorldLocation, GridLocation, ScatterDestination,
	                              DebugAStarMap);

	MovementPath = FPath(Path);
	MovementPath.DebugLog(TEXT("Scatter"));
	Target = FComputeTargetTileResult::Invalid();
}

void AGhostAIController::ComputeChasePath()
{
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto WorldLocation = Pawn->GetActorLocation2D();
	const auto GridLocation = Pawn->GetGridLocation();
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = FSafeGet::Pawn<AMovablePawn>(PlayerController);
	const auto PlayerGridLocation = PlayerPawn->GetGridLocation();
	const auto Path = ComputePath(ULevelLoader::GetInstance(Level), WorldLocation, GridLocation, PlayerGridLocation,
	                              DebugAStarMap);

	MovementPath = FPath(Path);
	MovementPath.DebugLog(TEXT("Chase"));
	Target = FComputeTargetTileResult::Invalid();
}
