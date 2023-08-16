#include "Controllers/GhostAiController.h"
#include "ChompGameState.h"
#include "ChompPlayerController.h"
#include "GhostHouseQueue.h"
#include "AStar/AStar.h"
#include "LevelGenerator/LevelLoader.h"
#include "Math/UnrealMathUtility.h"
#include "Pawns/ChompPawn.h"
#include "Pawns/GhostPawn.h"
#include "Pawns/MovablePawn.h"
#include "Pawns/Movement/MovementResult.h"
#include "Utils/ArrayHelpers.h"
#include "Utils/Debug.h"
#include "Utils/SafeGet.h"

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
	if (IsInGhostHouseQueue())
		return;

	// Early return if movement path hasn't been initialized yet.
	if (!MovementPath.IsValid())
		return;

	// Early return if player is dead.
	if (!IsPlayerAlive())
		return;

	// Preconditions.
	const auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
	{
		const auto ActorLocation = GhostPawn->GetActorLocation();
		checkf(!MovementPath.WasCompleted(ActorLocation), TEXT("Movement path mustn't be complete."));
	}

	// Compute new location and rotation.
	const auto GameSubstate = GameState->GetSubstateEnum();
	const auto MovementSpeed = GhostState == EGhostState::Eaten
		                           ? GhostPawn->GetReturnToGhostHouseMovementSpeed()
		                           : GhostState == EGhostState::Frightened
		                           ? GhostPawn->GetFrightenedMovementSpeed()
		                           : GhostState == EGhostState::Normal
		                           ? GhostPawn->GetMovementSpeed()
		                           : -1.0;
	checkf(MovementSpeed > 0, TEXT("GhostState must be handled by the ternary expression above: %d"), GhostState);
	const auto [NewLocation, NewRotation] = GhostPawn->MoveAlongPath(
		MovementPath,
		DeltaTime,
		MovementSpeed
	);

	// Apply new location. Player might be dead after this, so return early.
	GhostPawn->SetActorLocationAndRotation(NewLocation, NewRotation);
	if (!IsPlayerAlive())
	{
		return;
	}

	// Compute a new movement path if conditions are met.
	if (GhostState != EGhostState::Eaten)
	{
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
	}
	else if (GhostState == EGhostState::Eaten && MovementPath.WasCompleted(NewLocation))
	{
		// Find the underlying substate.
		const auto NonFrightenedSubstate = GameState->GetSubstateEnum(true);
		check(NonFrightenedSubstate != EChompPlayingSubstateEnum::Frightened);

		// Compute new movement path depending on underlying substate.
		if (NonFrightenedSubstate == EChompPlayingSubstateEnum::Scatter)
			MovementPath = UpdateMovementPathWhenInScatter();
		else if (NonFrightenedSubstate == EChompPlayingSubstateEnum::Chase)
			DecideToUpdateMovementPathInChase(NewLocation);
		else
			checkf(false, TEXT("Movement path is %d"), NonFrightenedSubstate);

		// Uncheck internal state for tracking whether ghost has been eaten.
		SetGhostState(EGhostState::Normal);
	}

#if WITH_EDITOR
	if (const auto WorldLocationPath = MovementPath.GetWorldLocationPath(); WorldLocationPath.Num() >= 1)
	{
		for (const auto SphereCenter : WorldLocationPath)
		{
			constexpr float SphereRadius = 25.0f; // Radius of the sphere
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

	{
		const auto GameState = FSafeGet::GameState<AChompGameState>(this);
		GameState->OnGamePlayingStateChangedDelegate.AddUniqueDynamic(
			this,
			&AGhostAiController::UpdateWhenSubstateChanges
		);
		GameState->OnDotsConsumedUpdatedDelegate.AddUniqueDynamic(
			this,
			&AGhostAiController::HandleDotsConsumedUpdated
		);
	}

	OnGhostStateChanged.AddUniqueDynamic(this, &AGhostAiController::HandleGhostStateChanged);
}

void AGhostAiController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	{
		const auto GameState = FSafeGet::GameState<AChompGameState>(this);
		GameState->OnGamePlayingStateChangedDelegate.RemoveDynamic(
			this,
			&AGhostAiController::UpdateWhenSubstateChanges
		);
		GameState->OnDotsConsumedUpdatedDelegate.RemoveDynamic(
			this,
			&AGhostAiController::HandleDotsConsumedUpdated
		);
	}

	OnGhostStateChanged.RemoveDynamic(this, &AGhostAiController::HandleGhostStateChanged);
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
void AGhostAiController::UpdateWhenSubstateChanges(EChompPlayingSubstateEnum OldState,
                                                   EChompPlayingSubstateEnum NewState)
{
	// Pre-conditions.
	check(OldState != NewState);
	DEBUG_LOG(TEXT("UpdateWhenSubstateChanges: %d to %d"), OldState, NewState);

	// Early returns.
	if (FSafeGet::GameState<AChompGameState>(this)->GetEnum() != EChompGameStateEnum::Playing)
		return;
	if (!IsPlayerAlive())
		return;

	if (GhostState != EGhostState::Eaten)
	{
		// Update ghost state.
		if (NewState == EChompPlayingSubstateEnum::Frightened)
			SetGhostState(EGhostState::Frightened);
		else if (NewState != EChompPlayingSubstateEnum::Frightened)
			SetGhostState(EGhostState::Normal);

		// Update movement path.
		if (NewState == EChompPlayingSubstateEnum::Scatter)
			MovementPath = UpdateMovementPathWhenInScatter();
		else if (NewState == EChompPlayingSubstateEnum::Chase)
			MovementPath = UpdateMovementPathWhenInChase();
		else if (NewState == EChompPlayingSubstateEnum::Frightened)
			MovementPath = UpdateMovementPathWhenInFrightened();
	}
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
	for (int X = LevelInstance->GetActualLevelHeight() - 1; X >= 0; X--)
	{
		FString Line = TEXT("");
		for (int Y = 0; Y < LevelInstance->GetActualLevelWidth(); Y++)
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
	// Pre-conditions.
	checkf(GhostState != EGhostState::Eaten,
	       TEXT("Ghosts in the eaten state should use a ReturnToGhostHouse movement path."));

	// Grab some references.
	const auto GhostPawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto GridLocation = GhostPawn->GetGridLocation();
	const auto DestinationNode = ComputeDestinationNodeInFrightened(GridLocation);

	const auto Path = ComputePath(
		ULevelLoader::GetInstance(Level),
		FVector2D(GhostPawn->GetActorLocation()),
		GridLocation,
		DestinationNode,
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

	// Reset the movement path too.
	MovementPath.Reset();
}

bool AGhostAiController::IsStartingPositionInGhostHouse() const
{
	const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto StartingPosition = Pawn->GetStartingPosition();
	return ULevelLoader::GetInstance(Level)->IsGhostHouse(StartingPosition);
}

bool AGhostAiController::IsInGhostHouseQueue() const
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

bool AGhostAiController::IsEaten() const
{
	return GhostState == EGhostState::Eaten;
}

bool AGhostAiController::IsNormal() const
{
	return GhostState == EGhostState::Normal;
}

void AGhostAiController::Consume()
{
	SetGhostState(EGhostState::Eaten);
	const auto ChompGameState = FSafeGet::GameState<AChompGameState>(this);
	ChompGameState->ConsumeGhost();
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
	return IsValid(PlayerPawn);
}

void AGhostAiController::SetGhostState(const EGhostState NewGhostState)
{
	GhostState = NewGhostState;
	OnGhostStateChanged.Broadcast(NewGhostState);
}

void AGhostAiController::HandleGhostStateChanged(const EGhostState NewGhostState)
{
	if (NewGhostState == EGhostState::Eaten)
		MovementPath = ReturnToGhostHouse();
}

FMovementPath AGhostAiController::ReturnToGhostHouse() const
{
	// Compute start position.
	const auto Pawn = FSafeGet::Pawn<AGhostPawn>(this);
	const auto StartPosition = Pawn->GetGridLocation();

	// Compute end position.
	const auto EndPosition = Pawn->GetGhostHouseReturnLocation();

	// Compute path.
	const auto WorldLocation = FVector2D(Pawn->GetActorLocation());
	const auto Path = ComputePath(
		ULevelLoader::GetInstance(Level),
		WorldLocation,
		StartPosition,
		EndPosition,
		DebugAStarMap
	);

	// Construct FMovementPath.
	const auto NewMovementPath = FMovementPath(Pawn->GetActorLocation(), Path, ULevelLoader::GetInstance(Level));
	NewMovementPath.DebugLog(TEXT("Chase"));

	// Post-conditions.
	check(NewMovementPath.IsValid());
	check(NewMovementPath != MovementPath);

	return NewMovementPath;
}

FGridLocation
AGhostAiController::ComputeDestinationNodeInFrightened(const FGridLocation& GridLocation) const
{
	const auto LevelInstance = ULevelLoader::GetInstance(Level);
	if (const auto IsGhostHouseTile =
		LevelInstance->IsGhostHouse(GridLocation) ||
		LevelInstance->IsGateTile(GridLocation))
	{
		// If we're in the ghost house, then return the point right outside of the ghost house.
		const auto Tile = LevelInstance->GetRightOutsideGhostHouseTile();
		return Tile;
	}

	// Save this for later.
	const auto MaxDimension = FMath::Max(
		ULevelLoader::GetInstance(Level)->GetLevelHeight(),
		ULevelLoader::GetInstance(Level)->GetLevelWidth()
	);

	// Find adjacent tiles, and jumble them.
	auto AdjacentTiles = ULevelLoader::GetInstance(Level)->Neighbors(GridLocation);
	FArrayHelpers::Randomize(AdjacentTiles);

	// Omit the direction that we came from.
	// ReSharper disable once CppTooWideScope
	bool DebugDidRemoveCameFrom = false;
	{
		const auto GridLocationPath = MovementPath.GetGridLocationPath();
		for (auto i = 1; i < GridLocationPath.Num(); i++)
		{
			if (GridLocation == GridLocationPath[i])
			{
				// Then get the previous node.
				const auto PrevNode = GridLocationPath[i - 1];

				// Remove it from AdjacentTiles.
				if (auto It = std::find(AdjacentTiles.begin(), AdjacentTiles.end(), PrevNode);
					It != AdjacentTiles.end())
				{
					DebugDidRemoveCameFrom = true;
					AdjacentTiles.erase(It);
					break;
				}
			}
		}
	}

	// Also omit the gate tile, since we do not want frightened ghosts to navigate into the ghost house.
	// Only eaten ghosts can navigate into the ghost house.
	// ReSharper disable once CppTooWideScope
	bool DebugDidRemoveGateTile = false;
	{
		// Disabling lint rule because it's a C++20 feature, which doesn't work in Unreal 5.2.
		// ReSharper disable once CppTooWideScopeInitStatement
		const auto GateTile = ULevelLoader::GetInstance(Level)->GetGateTile();
		if (const auto It = std::find(AdjacentTiles.begin(), AdjacentTiles.end(), GateTile);
			It != AdjacentTiles.end())
		{
			DebugDidRemoveGateTile = true;
			AdjacentTiles.erase(It);
		}
	}

	// Sanity check.
	checkf(AdjacentTiles.size() > 0, TEXT("Must have at least 1 adjacent tile."));

	// Iterate over tile choices.
	for (const auto& Tile : AdjacentTiles)
	{
		// Compute direction of adjacent tile.
		const auto [DirX, DirY] = Tile - GridLocation;

		// Iterate at most MaxDimension times in Dir.
		for (auto i = 1; i <= MaxDimension; i++)
		{
			const int X = FMath::RoundToInt(
				FMathHelpers::NegativeFriendlyFmod(DirX * i, LevelInstance->GetLevelHeight())
			);
			const int Y = FMath::RoundToInt(
				FMathHelpers::NegativeFriendlyFmod(DirX * i, LevelInstance->GetLevelWidth())
			);
			const auto Delta = FGridLocation{X, Y};
			const auto PossibleIntersectionTile = GridLocation + FGridLocation{DirX * i, DirY * i};

			if (ULevelLoader::GetInstance(Level)->IsWall(PossibleIntersectionTile))
				break;

			if (ULevelLoader::GetInstance(Level)->IsIntersectionTile(PossibleIntersectionTile))
				return PossibleIntersectionTile;
		}
	}

	checkf(false, TEXT("Did not find an intersection tile. Returning dummy value that will likely fail at runtime."))
	return FGridLocation{-1000, -1000};
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
