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

	if (IsTesting)
	{
		// Test out the one node path case.
		const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
		const auto StartingPosition = Pawn->GetStartingPosition();
		const std::vector OneNodePath{StartingPosition};
		MovementPath = FPath(OneNodePath);
	}

	// Attach some handlers for when game state changes.
	const auto GameState = FSafeGet::GameState<AChompGameState>(this);
	GameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(
		this,
		&AGhostAIController::HandleGamePlayingSubstateChanged);
	GameState->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleGameStateChanged);
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

	// Update movement path if needed.
	const auto CurrentLocationIndex = MovementPath.GetCurrentLocationIndex();
	auto CurrentMoveDirection = MovementPath.GetCurrentMoveDirection(ActorLocation, LevelInstance);
	if (CurrentLocationIndex == -1 && CurrentMoveDirection.IsZero())
	{
		MovementPath.Increment();
		CurrentMoveDirection = MovementPath.GetCurrentMoveDirection(ActorLocation, LevelInstance);
	}

	// Update target if needed.
	if (!MovementPath.WasCompleted() && !Target.IsValid)
		Target = FComputeTargetTileResult{true, MovementPath.GetTargetLocation()};

	// If there is no target, early return.
	if (!Target.IsValid)
		return;

	// Else, move toward the target.
	if (const auto [MovedPastTarget, AmountMovedPast] = MovablePawn->MoveTowardsPoint(
		Target.Tile, CurrentMoveDirection, DeltaTime, FName("AI")); MovedPastTarget)
	{
		// Grab the old direction before updating the movement path.
		const auto OldDir = MovementPath.GetCurrentMoveDirection(MovablePawn->GetActorLocation(), LevelInstance);

		// Update the movement path.
		// Note that the order of operations is important here.
		MovementPath.Increment();
		if (PlayingSubstate == EChompGamePlayingSubstate::Scatter && MovementPath.WasCompleted())
		{
			auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
			auto Destination = Pawn->GetScatterDestination();
			ComputeScatterForMovementPath(Destination);
			Pawn->SwapScatterOriginAndDestination();
		}
		else if (PlayingSubstate == EChompGamePlayingSubstate::Chase && MovementPath.WasCompleted(0))
		{
			ComputeChaseForMovementPath();
		}

		// Update the target.
		// Note that the target world pos is fetched before clearing.
		const auto TargetWorldPos = LevelInstance->GridToWorld(Target.Tile);
		Target = FComputeTargetTileResult::Invalid();

		// Apply movement.
		{
			const auto NewDir = MovementPath.GetCurrentMoveDirection(MovablePawn->GetActorLocation(), LevelInstance);
			const auto ActorLocation2 = MovablePawn->GetActorLocation();
			FVector NewLocation =
				OldDir == NewDir
					?
					// Maintain the extra amount in the existing move direction.
					FVector{
						TargetWorldPos.X + OldDir.X * AmountMovedPast,
						TargetWorldPos.Y + OldDir.Y * AmountMovedPast,
						0.0f
					}
					:
					// Do not apply any extra amount in the new move direction.
					FVector{
						TargetWorldPos.X,
						TargetWorldPos.Y,
						0.0f
					};
			const FVector Difference = NewLocation - ActorLocation2;
			check(Difference.Size() < 0.5f * 100.0f); // A half-unit jump is sketchy.
			MovablePawn->SetActorLocation(NewLocation);
		}
	}
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
	check(Path[0] == StartGridPos);
	check(Path.size() >= 1);

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

void AGhostAIController::ComputeScatterForMovementPath(const FGridLocation& ScatterDestination)
{
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto WorldLocation = Pawn->GetActorLocation2D();
	const auto GridLocation = Pawn->GetGridLocation();
	const auto Path = ComputePath(ULevelLoader::GetInstance(Level), WorldLocation, GridLocation, ScatterDestination,
	                              DebugAStarMap);

	MovementPath = FPath(Path);
	MovementPath.DebugLog(TEXT("Scatter"));
}

void AGhostAIController::ComputeChaseForMovementPath()
{
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto WorldLocation = Pawn->GetActorLocation2D();
	const auto GridLocation = Pawn->GetGridLocation();
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = FSafeGet::Pawn<AMovablePawn>(PlayerController);
	const auto PlayerWorldPosition = PlayerPawn->GetActorLocation();
	DEBUG_LOG(TEXT("%s"), *PlayerWorldPosition.ToString());
	const auto PlayerGridLocation = PlayerPawn->GetGridLocation();
	const auto Path = ComputePath(ULevelLoader::GetInstance(Level), WorldLocation, GridLocation, PlayerGridLocation,
	                              DebugAStarMap);

	MovementPath = FPath(Path);
	MovementPath.DebugLog(TEXT("Chase"));
}
