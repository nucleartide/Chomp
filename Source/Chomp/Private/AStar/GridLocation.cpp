#include "AStar/GridLocation.h"
#include "LevelGenerator/LevelLoader.h"

bool operator ==(const FGridLocation& A, const FGridLocation& B)
{
	return A.X == B.X && A.Y == B.Y;
}

bool operator!=(const FGridLocation& A, const FGridLocation& B)
{
	return !(A == B);
}

bool operator<(const FGridLocation& A, const FGridLocation& B)
{
	return std::tie(A.X, A.Y) < std::tie(B.X, B.Y);
}

FString FGridLocation::ToString() const
{
	return FString::Printf(TEXT("(%d, %d)"), X, Y);
}

bool FGridLocation::IsNonZero() const
{
	return !IsZero();
}

bool FGridLocation::IsZero() const
{
	return X == 0 && Y == 0;
}

FGridLocation FGridLocation::operator+(const FGridLocation& IntendedDir) const
{
	return FGridLocation{X + IntendedDir.X, Y + IntendedDir.Y};
}

FGridLocation FGridLocation::operator-(const FGridLocation& IntendedDir) const
{
	return FGridLocation{X - IntendedDir.X, Y - IntendedDir.Y};
}

FVector NormalizeForWrapAround(const FVector& Vec, const ILevelLoader* LevelInstance)
{
	const auto LevelWorldWidth = LevelInstance->GetLevelWidth() * 100.0;
	const auto LevelWorldHeight = LevelInstance->GetLevelHeight() * 100.0;
	return FVector(
		Vec.X < 0.0 ? Vec.X + LevelWorldHeight : Vec.X,
		Vec.Y < 0.0 ? Vec.Y + LevelWorldWidth : Vec.Y,
		0.0
	);
}

// Return whether a Location is in the range [A, B), and by how much.
std::optional<double> FGridLocation::IsInBetween(
	const FVector& Location,
	const FVector& A,
	const FVector& B,
	const ILevelLoader* LevelInstance)
{
	// Pre-conditions.
	checkf(
		(
			FMath::Abs(A.X - B.X) <= 100.0 ||
			FMath::Abs(A.X - B.X) >= (LevelInstance->GetLevelHeight() - 1) * 100.0
		) &&
		(
			FMath::Abs(A.Y - B.Y) <= 100.0 ||
			FMath::Abs(A.Y - B.Y) >= (LevelInstance->GetLevelWidth() - 1) * 100.0
		),
		TEXT("A and B are within one unit of each other.")
	);

	const auto XDiff = FMath::Abs(A.X - B.X);
	const auto YDiff = FMath::Abs(A.Y - B.Y);
	FVector NLoc;
	FVector Na;
	FVector Nb;

	if (
		XDiff >= (LevelInstance->GetLevelHeight() - 1) * 100.0 ||
		YDiff >= (LevelInstance->GetLevelWidth() - 1) * 100.0)
	{
		NLoc = NormalizeForWrapAround(Location, LevelInstance);
		Na = NormalizeForWrapAround(A, LevelInstance);
		Nb = NormalizeForWrapAround(B, LevelInstance);
	}
	else
	{
		NLoc = Location;
		Na = A;
		Nb = B;
	}

	if (const auto IsMovingOnXAxis =
		FMath::IsNearlyEqual(Na.X, Nb.X) &&
		FMath::IsNearlyEqual(NLoc.X, Na.X))
	{
		if (Na.Y < Nb.Y &&
			Na.Y <= NLoc.Y && NLoc.Y < Nb.Y)
		{
			return NLoc.Y - Na.Y;
		}

		if (Nb.Y < Na.Y &&
			Nb.Y < NLoc.Y && NLoc.Y <= Na.Y)
		{
			return Na.Y - NLoc.Y;
		}
	}

	if (const auto IsMovingOnYAxis =
		FMath::IsNearlyEqual(Na.Y, Nb.Y) &&
		FMath::IsNearlyEqual(NLoc.Y, Na.Y))
	{
		if (Na.X < Nb.X &&
			Na.X <= NLoc.X && NLoc.X < Nb.X)
		{
			return NLoc.X - Na.X;
		}

		if (Nb.X < Na.X &&
			Nb.X < NLoc.X && NLoc.X <= Na.X)
		{
			return Na.X - NLoc.X;
		}
	}

	return std::nullopt;
}

FGridLocation FGridLocation::Modulo(const ULevelLoader* LevelInstance) const
{
	const auto Height = LevelInstance->GetLevelHeight();
	const auto Width = LevelInstance->GetLevelWidth();

	auto NewX = X;
	if (NewX < 0)
		NewX += Height;
	else if (NewX >= Height)
		NewX -= Height;

	auto NewY = Y;
	if (NewY < 0)
		NewY += Width;
	else if (NewY >= Width)
		NewY -= Width;

	return FGridLocation{NewX, NewY};
}
