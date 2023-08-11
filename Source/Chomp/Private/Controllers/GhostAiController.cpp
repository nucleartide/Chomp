#include "Controllers/GhostAiController.h"

#include "GhostHouseQueue.h"
#include "Math/UnrealMathUtility.h"
#include "AStar/AStar.h"
#include "ChompGameState.h"
#include "ChompPlayerController.h"
#include "LevelGenerator/LevelLoader.h"
#include "Pawns/ChompPawn.h"
#include "Pawns/GhostPawn.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"
#include "Pawns/MovablePawn.h"
#include "Pawns/Movement/MovementResult.h"
#include "Utils/ArrayHelpers.h"

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
	if (GameState->GetEnum() != EChompGameStateEnum::Playing)
		return;

	// Early return if in the ghost house.
	if (IsInGhostHouse())
		return;

	// Early return if movement path hasn't been initialized yet.
	if (!MovementPath.IsValid())
		return;

	// Early return if player is dead.
	if (!IsPlayerAlive())
		return;

	// Preconditions.
	const auto MovablePawn = FSafeGet::Pawn<AMovablePawn>(this);
	{
		const auto ActorLocation = MovablePawn->GetActorLocation();
	checkf(!MovementPath.WasCompleted(ActorLocation), TEXT("Movement path mustn't be complete."));
	}

	// Compute new location and rotation.
	const auto [NewLocation, NewRotation] = MovablePawn->MoveAlongPath(
		MovementPath,
		DeltaTime
	);

	// Apply new location.
	MovablePawn->SetActorLocationAndRotation(NewLocation, NewRotation);

	// Compute a new movement path if conditions are met.
	if (const auto PlayingSubstate = GameState->GetSubstateEnum();
		PlayingSubstate == EChompPlayingSubstateEnum::Scatter &&
		MovementPath.WasCompleted(NewLocation))
	{
		MovementPath = UpdateMovementPathWhenInScatter();
	}
	else if (PlayingSubstate == EChompPlayingSubstateEnum::Chase)
	{
		DecideToUpdateMovementPathInChase(NewLocation);
	}
	else if (
		PlayingSubstate == EChompPlayingSubstateEnum::Frightened &&
		MovementPath.WasCompleted(NewLocation))
	{
		MovementPath = UpdateMovementPathWhenInFrightened();
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

FGridLocation AGhostAiController::GetPlayerGridLocation() const
{
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AChompPawn>();
	checkf(PlayerPawn,
	       TEXT("Player must be alive, otherwise we wouldn't be recomputing paths."));
	return PlayerPawn->GetGridLocation();
}

FGridLocation AGhostAiController::GetPlayerGridDirection() const
{
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto ChompPlayerController = Cast<AChompPlayerController>(PlayerController);
	check(ChompPlayerController);
	const auto PlayerGridDirection = ChompPlayerController->GetCurrentMovement();
	return PlayerGridDirection;
}

FVector AGhostAiController::GetPlayerWorldLocation() const
{
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AChompPawn>();
	checkf(PlayerPawn,
	       TEXT("Player must be alive, otherwise we wouldn't be be recomputing paths."));
	return PlayerPawn->GetActorLocation();
}

/**
 * Sync the GhostAIController with the playing sub-state of the game.
 */
// ReSharper disable once CppMemberFunctionMayBeStatic
void AGhostAiController::HandleGamePlayingSubstateChanged(EChompPlayingSubstateEnum OldState,
                                                          EChompPlayingSubstateEnum NewState)
{
	// Pre-conditions.
	check(OldState != NewState);
	DEBUG_LOG(TEXT("HandleGamePlayingSubstateChanged: %d to %d"), OldState, NewState);

	// Early returns.
	if (FSafeGet::GameState<AChompGameState>(this)->GetEnum() != EChompGameStateEnum::Playing)
		return;
	if (!IsPlayerAlive())
		return;

	// TODO: Update movement path computation so that moving within the ghosthouse is allowed.
	if (NewState == EChompPlayingSubstateEnum::Scatter)
		MovementPath = UpdateMovementPathWhenInScatter();
	else if (NewState == EChompPlayingSubstateEnum::Chase)
		MovementPath = UpdateMovementPathWhenInChase();
	else if (NewState == EChompPlayingSubstateEnum::Frightened)
		MovementPath = UpdateMovementPathWhenInFrightened();
}

// ReSharper disable once CppMemberFunctionMayBeConst
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
void AGhostAiController::HandleGameStateChanged(EChompGameStateEnum OldState, EChompGameStateEnum NewState)
{
	check(OldState != NewState);
	if (NewState == EChompGameStateEnum::Playing)
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

FMovementPath AGhostAiController::UpdateMovementPathWhenInScatter()
{
	// Grab some values.
	const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto WorldLocation = FVector2D(Pawn->GetActorLocation());
	const auto GridLocation = Pawn->GetGridLocation();

	// If we are on the CurrentScatterDestination, swap so we don't compute a 1-node path.
	if (GridLocation == CurrentScatterDestination)
		std::swap(CurrentScatterOrigin, CurrentScatterDestination);

	// If we are computing the same path, then return early to avoid the post-condition check.
	if (const auto GridLocationPath = MovementPath.GetGridLocationPath();
		GridLocationPath.Num() > 0)
	{
		const auto First = GridLocationPath[0];
		// ReSharper disable once CppTooWideScopeInitStatement
		const auto Last = GridLocationPath[GridLocationPath.Num() - 1];
		if (First == GridLocation && Last == CurrentScatterDestination)
			return MovementPath;
	}

	const auto Path = ComputePath(
		ULevelLoader::GetInstance(Level),
		WorldLocation,
		GridLocation,
		CurrentScatterDestination,
		DebugAStarMap
	);

	const auto NewMovementPath = FMovementPath(Pawn->GetActorLocation(), Path, ULevelLoader::GetInstance(Level));
	NewMovementPath.DebugLog(TEXT("Scatter"));

	// Post-conditions.
	check(NewMovementPath.IsValid());
	check(NewMovementPath != MovementPath);

	// Swap the scatter origin and destination for the next time.
	std::swap(CurrentScatterOrigin, CurrentScatterDestination);

	return NewMovementPath;
}

FMovementPath AGhostAiController::UpdateMovementPathWhenInFrightened() const
{
	const auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto GridLocation = GhostPawn->GetGridLocation();

	// Jumble adjacent tiles.
	auto AdjacentTiles = ULevelLoader::GetInstance(Level)->Neighbors(GridLocation);
	FArrayHelpers::Randomize(AdjacentTiles);

	// Find the intersection tile in our selected direction.
	const auto MaxDimension = FMath::Max(
		ULevelLoader::GetInstance(Level)->GetLevelHeight(),
		ULevelLoader::GetInstance(Level)->GetLevelWidth()
	);

	// Iterate over tile choices.
	for (const auto& Tile : AdjacentTiles)
	{
		// Compute direction of adjacent tile.
		const auto [DirX, DirY] = Tile - GridLocation;

		// Iterate at most MaxDimension times in Dir.
		for (auto i = 1; i <= MaxDimension; i++)
		{
			const auto PossibleIntersectionTile = GridLocation + FGridLocation{DirX * i, DirY * i};

			if (ULevelLoader::GetInstance(Level)->IsWall(PossibleIntersectionTile))
				break;
			
			if (ULevelLoader::GetInstance(Level)->IsIntersectionTile(PossibleIntersectionTile))
			{
				const auto Path = ComputePath(
					ULevelLoader::GetInstance(Level),
					FVector2D(GhostPawn->GetActorLocation()),
					GridLocation,
					PossibleIntersectionTile,
					DebugAStarMap
				);

				const auto NewMovementPath = FMovementPath(
					GhostPawn->GetActorLocation(),
					Path,
					ULevelLoader::GetInstance(Level)
				);

				NewMovementPath.DebugLog(TEXT("Frightened"));

				return NewMovementPath;
			}
		}
	}

	checkf(false, TEXT("Could not find an intersection node."));
	return MovementPath;
}

FMovementPath AGhostAiController::UpdateMovementPathWhenInChase() const
{
	// Preconditions.
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AMovablePawn>();
	checkf(PlayerPawn, TEXT("Player is alive"));

	// Compute start position.
	const auto Pawn = FSafeGet::Pawn<AMovablePawn>(this);
	const auto StartPosition = Pawn->GetGridLocation();

	// Compute end position.
	// If it's the same as the current end position, then force the end position to be the player instead.
	auto EndPosition = GetChaseEndGridPosition();
	if (const auto GridLocationPath = MovementPath.GetGridLocationPath();
		EndPosition == GridLocationPath[GridLocationPath.Num() - 1])
		EndPosition = PlayerPawn->GetGridLocation();

	// If we are computing the same path, then return early to avoid the post-condition check.
	if (const auto GridLocationPath = MovementPath.GetGridLocationPath();
		GridLocationPath.Num() > 0)
	{
		const auto First = GridLocationPath[0];
		// ReSharper disable once CppTooWideScopeInitStatement
		const auto Last = GridLocationPath[GridLocationPath.Num() - 1];
		if (First == StartPosition && Last == EndPosition)
			return MovementPath;
	}

	// Compute path.
	const auto WorldLocation = FVector2D(Pawn->GetActorLocation());
	const auto Path = ComputePath(
		ULevelLoader::GetInstance(Level),
		WorldLocation,
		StartPosition,
		EndPosition,
		DebugAStarMap
	);

	// Update movement path.
	const auto NewMovementPath = FMovementPath(Pawn->GetActorLocation(), Path, ULevelLoader::GetInstance(Level));
	NewMovementPath.DebugLog(TEXT("Chase"));

	// Post-conditions.
	check(NewMovementPath.IsValid());
	check(NewMovementPath != MovementPath);

	return NewMovementPath;
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

bool AGhostAiController::IsPlayerAlive() const
{
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AMovablePawn>();
	return PlayerPawn != nullptr;
}

void AGhostAiController::DecideToUpdateMovementPathInChase_Implementation(const FVector NewLocation)
{
	if (
		MovementPath.GetWorldLocationPath().Num() >= 2 && MovementPath.DidComplete(NewLocation, 1) ||
		MovementPath.GetWorldLocationPath().Num() == 1 && MovementPath.WasCompleted(NewLocation)
	)
	{
		// ReSharper disable once CppDeclaratorNeverUsed
		const auto DebugA = MovementPath.GetWorldLocationPath().Num();
		// ReSharper disable once CppDeclaratorNeverUsed
		const auto DebugB = MovementPath.DidComplete(NewLocation, 1);
		// ReSharper disable once CppDeclaratorNeverUsed
		const auto DebugC = MovementPath.WasCompleted(NewLocation);
		MovementPath = UpdateMovementPathWhenInChase();
	}
}

FGridLocation AGhostAiController::GetChaseEndGridPosition_Implementation() const
{
	const auto PlayerController = FSafeGet::PlayerController(this, 0);
	const auto PlayerPawn = PlayerController->GetPawn<AMovablePawn>();
	checkf(PlayerPawn, TEXT("Player is alive"));
	return PlayerPawn->GetGridLocation();
}
