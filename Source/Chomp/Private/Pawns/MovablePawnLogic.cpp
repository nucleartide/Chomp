#include "MovablePawnLogic.h"

UMovablePawnLogic::UMovablePawnLogic()
{
}

UMovablePawnLogic::UMovablePawnLogic(ULevelLoader* LevelInstance): LevelInstance(LevelInstance)
{
}

FMoveTowardsPointResult UMovablePawnLogic::MoveTowardsPoint(
	const FGridLocation& TargetGridPosition,
	const FGridLocation& TargetDirection,
	float DeltaTime,
	FVector Location,
	FRotator Rotation)
{
	constexpr FMoveTowardsPointResult Result{true, 42};
	// TODO: Need to return a NewLocation, given the input args & wrap-around behavior.
	// TODO: Need to return a NewRotation.
	return Result;
}
