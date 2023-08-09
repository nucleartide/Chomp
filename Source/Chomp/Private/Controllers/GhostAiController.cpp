#include "Controllers/GhostAiController.h"

#include "GhostHouseQueue.h"
#include "Math/UnrealMathUtility.h"
#include "AStar/AStar.h"
#include "ChompGameState.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"
#include "Pawns/MovablePawn.h"
#include "Pawns/Movement/MovementResult.h"

void AGhostAiController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ResetGhostState();
}

void AGhostAiController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Early return if not playing.
	const auto GameState = FSafeGet::GameState<AChompGameState>(this);
	if (GameState->GetEnum() != EChompGameState::Playing)
		return;

	// Early return if in the ghost house.
	if (IsInGhostHouse())
		return;

	// Early return if movement path hasn't been initialized yet.
	if (!MovementPath.IsValid())
		return;

	// Preconditions.
	const auto MovablePawn = FSafeGet::Pawn<AMovablePawn>(this);
	checkf(!MovementPath.WasCompleted(MovablePawn->GetActorLocation()), TEXT("Movement path mustn't be complete."));

	// Compute new location and rotation.
	const auto [NewLocation, NewRotation] = MovablePawn->MoveAlongPath(
		MovementPath,
		DeltaTime
	);

	// Apply new location.
	MovablePawn->SetActorLocationAndRotation(NewLocation, NewRotation);

	// Compute a new movement path if conditions are met.
	if (const auto PlayingSubstate = GameState->GetPlayingSubstate();
		PlayingSubstate == EChompGamePlayingSubstate::Scatter &&
		MovementPath.WasCompleted(NewLocation))
	{
		const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
		const auto Destination = Pawn->GetScatterDestination();
		UpdateMovementPathWhenInScatter(Destination);
		SwapScatterOriginAndDestination();
	}
	else if (
		PlayingSubstate == EChompGamePlayingSubstate::Chase &&
		(
			MovementPath.GetWorldLocationPath().Num() >= 2 && MovementPath.DidComplete(NewLocation, 1) ||
			MovementPath.GetWorldLocationPath().Num() == 1 && MovementPath.WasCompleted(NewLocation)
		)
	)
	{
		const auto DebugA = MovementPath.GetWorldLocationPath().Num() == 1;
		const auto DebugB = MovementPath.WasCompleted(NewLocation);
		const auto DebugC = FSafeGet::PlayerController(this, 0);
		const auto DebugD = DebugC->GetPawn<AMovablePawn>();
		const auto DidUpdate = UpdateMovementPathWhenInChase();
		checkf(DidUpdate, TEXT("If false, check whether the player pawn is destroyed."));
	}

#if WITH_EDITOR
	if (const auto WorldLocationPath = MovementPath.GetWorldLocationPath(); WorldLocationPath.Num() >= 1)
	{
		for (const auto SphereCenter : WorldLocationPath)
		{
			constexpr float SphereRadius = 25.0f; // Radius of the sphere
			const auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
			const auto SphereColor = GhostPawn->GetDebugColor().ToFColor(false);

			// Draw the debug sphere
			DrawDebugSphere(
				GetWorld(),
				SphereCenter,
				SphereRadius,
				12,
				SphereColor,
				false,
				-1.0,
				0,
				4.0);
		}
	}
#endif
}

void AGhostAiController::BeginPlay()
{
	Super::BeginPlay();

	// Attach some handlers for when game state changes.
	{
		const auto GameState = FSafeGet::GameState<AChompGameState>(this);
		GameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(
			this,
			&AGhostAiController::HandleGamePlayingSubstateChanged
		);
		GameState->OnDotsConsumedUpdatedDelegate.AddUniqueDynamic(
			this,
			&AGhostAiController::HandleDotsConsumedUpdated
		);
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
		UpdateMovementPathWhenInScatter(Destination);
	}
	else if (NewState == EChompGamePlayingSubstate::Chase)
	{
		UpdateMovementPathWhenInChase();
	}
}

void AGhostAiController::HandleDotsConsumedUpdated(const int NewDotsConsumed)
{
	const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	if (
		const auto Threshold = Pawn->GetDotsConsumedMovementThreshold();
		Threshold >= 0 && NewDotsConsumed >= Threshold)
	{
		const auto Queue = Pawn->GetGhostHouseQueue();
		Queue->Remove(this);
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
		ResetGhostState();
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

void AGhostAiController::UpdateMovementPathWhenInScatter(const FGridLocation& ScatterDestination)
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

bool AGhostAiController::UpdateMovementPathWhenInChase()
{
	// Compute start position.
	const auto [IsValid, GridLocation] = GetChaseStartGridPosition();
	check(IsValid);

	// Compute end position.
	const auto EndPosition = GetChaseEndGridPosition();
	if (!EndPosition.IsValid)
		return false;

	// Compute path.
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto WorldLocation = FVector2D(Pawn->GetActorLocation());
	const auto Path = ComputePath(
		ULevelLoader::GetInstance(Level),
		WorldLocation,
		GridLocation,
		EndPosition.GridLocation,
		DebugAStarMap
	);

	// Update movement path.
	MovementPath = FMovementPath(Pawn->GetActorLocation(), Path, ULevelLoader::GetInstance(Level));
	check(MovementPath.IsValid());
	MovementPath.DebugLog(TEXT("Chase"));
	return true;
}

void AGhostAiController::ResetGhostState()
{
	// Set the starting position of the pawn.
	const auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto StartingGridPosition = GhostPawn->GetStartingPosition();
	const auto StartingWorldPosition = ULevelLoader::GetInstance(Level)->GridToWorld(StartingGridPosition);
	const FVector StartingWorldPos(StartingWorldPosition.X, StartingWorldPosition.Y, 0.0f);
	GetPawn()->SetActorLocation(StartingWorldPos);

	// Add this instance of AGhostAiController to our RuntimeSet if not added already.
	if (IsStartingPositionInGhostHouse() && !GetGhostHouseQueue()->Contains(this))
	{
		GetGhostHouseQueue()->Add(this);
	}

	// Initialize CurrentScatterOrigin and CurrentScatterDestination from Pawn.
	{
		const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
		CurrentScatterOrigin = Pawn->GetScatterOrigin();
		CurrentScatterDestination = Pawn->GetScatterDestination();
	}
}

void AGhostAiController::SwapScatterOriginAndDestination()
{
	const FGridLocation Swap{CurrentScatterOrigin.X, CurrentScatterOrigin.Y};
	CurrentScatterOrigin = CurrentScatterDestination;
	CurrentScatterDestination = Swap;
}

bool AGhostAiController::IsStartingPositionInGhostHouse() const
{
	const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto StartingPosition = Pawn->GetStartingPosition();
	return ULevelLoader::GetInstance(Level)->IsGhostHouse(StartingPosition);
}

bool AGhostAiController::IsInGhostHouse() const
{
	const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto Queue = Pawn->GetGhostHouseQueue();
	return Queue->Contains(this);
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

FMaybeGridLocation AGhostAiController::GetChaseStartGridPosition_Implementation() const
{
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	return FMaybeGridLocation{true, Pawn->GetGridLocation()};
}

FMaybeGridLocation AGhostAiController::GetChaseEndGridPosition_Implementation() const
{
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AMovablePawn>();
	if (!PlayerPawn)
		return FMaybeGridLocation::Invalid();

	return FMaybeGridLocation{true, PlayerPawn->GetGridLocation()};
}
