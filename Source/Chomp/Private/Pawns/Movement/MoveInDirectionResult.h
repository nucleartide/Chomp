#pragma once

struct FMoveInDirectionResult
{
	FVector NewLocation;
	FRotator NewRotation;
	bool InvalidateTargetTile;
	bool CanTravelInIntendedDirection;

	FMoveInDirectionResult(const FVector& NewLocation, const FRotator& NewRotation, const bool bInvalidateTargetTile,
	                       const bool CanTravelInIntendedDirection)
		: NewLocation(NewLocation),
		  NewRotation(NewRotation),
		  InvalidateTargetTile(bInvalidateTargetTile),
		  CanTravelInIntendedDirection(CanTravelInIntendedDirection)
	{
	}
};
