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

	// Initialize MovementPath to a one-node path.
	if (IsTesting)
	{
		// Reset pawn position before performing test.
		ResetPawnPosition();

		// Construct test movement path.
		const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
		const auto StartingPosition = Pawn->GetStartingPosition();
		const std::vector OneNodePath{StartingPosition};
		MovementPath = MakeShared<FMovementPath>(Pawn->GetActorLocation(), OneNodePath, ULevelLoader::GetInstance(Level));
		check(MovementPath.IsValid());
	}

	// Attach some handlers for when game state changes.
	const auto GameState = FSafeGet::GameState<AChompGameState>(this);
	GameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(
		this,
		&AGhostAIController::HandleGamePlayingSubstateChanged);
	GameState->OnGameStateChangedDelegate.AddUniqueDynamic(this, &AGhostAIController::HandleGameStateChanged);

	// Idea for debugging if you need it:
	// InputComponent->BindAction("Perform", IE_Pressed, this, &AGhostAIController::OnEKeyPressed);
}

void AGhostAIController::Tick(float DeltaTime)
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

	// Grab references to things.
	auto PlayingSubstate = GameState->GetPlayingSubstate();
	auto LevelInstance = ULevelLoader::GetInstance(Level);
	auto MovablePawn = FSafeGet::Pawn<AMovablePawn>(this);
	auto ActorLocation = MovablePawn->GetActorLocation();
	FVector2D ActorLocation2D{ActorLocation.X, ActorLocation.Y};
	auto TagsToCollideWith = MovablePawn->GetTagsToCollideWith();
	auto GridLocation = LevelInstance->WorldToGrid(ActorLocation2D);
	check(MovementPath.IsValid());
	const auto MovementPathPtr = MovementPath.Get();

	// Compute new location and rotation.
	const auto [NewLocation, NewRotation] = MovablePawn->MoveTowardsPoint2(
		ActorLocation,
		MovablePawn->GetActorRotation(),
		MovementPathPtr,
		DeltaTime);
	
	// Apply new location.
	MovablePawn->SetActorLocationAndRotation(NewLocation, NewRotation);
	
#if false
	// After location and rotation have been applied, compute a new path if condition has been met.
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
		ResetPawnPosition();
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

	auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	return Threshold >= 0 &&
		NumberOfDotsConsumed >= 0 &&
		NumberOfDotsConsumed >= Threshold &&
		!MovementPath->WasCompleted(Pawn->GetActorLocation());
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

	MovementPath = MakeShared<FMovementPath>(Pawn->GetActorLocation(), Path, ULevelLoader::GetInstance(Level));
	check(MovementPath.IsValid());
	MovementPath->DebugLog(TEXT("Scatter"));
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

	MovementPath = MakeShared<FMovementPath>(Pawn->GetActorLocation(), Path, ULevelLoader::GetInstance(Level));
	check(MovementPath.IsValid());
	MovementPath->DebugLog(TEXT("Chase"));
}

void AGhostAIController::ResetPawnPosition() const
{
	// Set the starting position of the pawn.
	const auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto StartingGridPosition = GhostPawn->GetStartingPosition();
	const auto StartingWorldPosition = ULevelLoader::GetInstance(Level)->GridToWorld(StartingGridPosition);
	const FVector StartingWorldPos(StartingWorldPosition.X, StartingWorldPosition.Y, 0.0f);
    GetPawn()->SetActorLocation(StartingWorldPos);
}
