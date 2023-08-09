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

// Return whether a Location is in the range [WorldA, WorldB), and by how much.
std::optional<double> FGridLocation::IsInBetween(
	const FVector& Location,
	const FVector& WorldA,
	const FVector& WorldB)
{
	if (const auto IsMovingOnXAxis =
		FMath::IsNearlyEqual(WorldA.X, WorldB.X) &&
		FMath::IsNearlyEqual(Location.X, WorldA.X))
	{
		if (WorldA.Y < WorldB.Y &&
			WorldA.Y <= Location.Y && Location.Y < WorldB.Y)
			return Location.Y - WorldA.Y;
	
		if (WorldB.Y < WorldA.Y &&
			WorldB.Y < Location.Y && Location.Y <= WorldA.Y)
			return WorldA.Y - Location.Y;
	}

	if (const auto IsMovingOnYAxis =
		FMath::IsNearlyEqual(WorldA.Y, WorldB.Y) &&
		FMath::IsNearlyEqual(Location.Y, WorldA.Y))
	{
		if (WorldA.X < WorldB.X &&
			WorldA.X <= Location.X && Location.X < WorldB.X)
			return Location.X - WorldA.X;
		
		if (WorldB.X < WorldA.X &&
			WorldB.X < Location.X && Location.X <= WorldA.X)
			return WorldA.X - Location.X;
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
