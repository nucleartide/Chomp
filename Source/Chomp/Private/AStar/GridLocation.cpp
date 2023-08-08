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

bool FGridLocation::IsInBetween(const FVector& Location, const FGridLocation& A, const FGridLocation& B,
                                const ULevelLoader* LevelInstance)
{
	const auto WorldA = LevelInstance->GridToWorld(A);
	const auto WorldB = LevelInstance->GridToWorld(B);

	if (FMath::IsNearlyEqual(WorldA.X, WorldB.X) && FMath::IsNearlyEqual(Location.X, WorldA.X))
	{
		if (WorldA.Y < WorldB.Y && WorldA.Y <= Location.Y && Location.Y < WorldB.Y)
			return true;
		if (WorldB.Y < WorldA.Y && WorldB.Y < Location.Y && Location.Y <= WorldA.Y)
			return true;
	}

	if (FMath::IsNearlyEqual(WorldA.Y, WorldB.Y) && FMath::IsNearlyEqual(Location.Y, WorldA.Y))
	{
		if (WorldA.X < WorldB.X && WorldA.X <= Location.X && Location.X < WorldB.X)
			return true;
		if (WorldB.X < WorldA.X && WorldB.X < Location.X && Location.X <= WorldA.X)
			return true;
	}

	return false;
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
