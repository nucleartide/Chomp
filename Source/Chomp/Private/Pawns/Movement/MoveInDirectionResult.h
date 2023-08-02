#pragma once

struct FMoveInDirectionResult
{
	FVector NewLocation;
	FRotator NewRotation;
	bool InvalidateTargetTile;

	FMoveInDirectionResult(const FVector& NewLocation, const FRotator& NewRotation, const bool bInvalidateTargetTile)
		: NewLocation(NewLocation),
		  NewRotation(NewRotation),
		  InvalidateTargetTile(bInvalidateTargetTile)
	{
	}
};
