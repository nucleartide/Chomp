#include "LevelGenerator/LevelLoader.h"
#include <algorithm>
#include "Algo/Reverse.h"
#include "Constants/ChompGameplayTag.h"
#include "Engine/World.h"

ULevelLoader* ULevelLoader::GetInstance(const TSubclassOf<ULevelLoader>& BlueprintClass)
{
	auto DefaultObject = BlueprintClass.GetDefaultObject();
	check(DefaultObject);

	auto Level = Cast<ULevelLoader>(DefaultObject);
	check(Level);

	return Level;
}

void ULevelLoader::LoadLevel()
{
	// Load file into variable.
	TArray<uint8> FileData;
	FString FilePath = FPaths::ProjectContentDir() + LevelFilename; // Example: "Levels/level2.txt"
	auto Ok = FFileHelper::LoadFileToArray(FileData, *FilePath);
	check(Ok);

	// The FileContent FString now contains the text content of the loaded file.
	FString FileContent{""};
	FFileHelper::BufferToString(FileContent, FileData.GetData(), FileData.Num());

	// Split FileContent into a list of lines.
	FileContent.ParseIntoArray(StringList, TEXT("\n"), true);

	// Save the number of lines.
	NumberOfRows = StringList.Num();
	check(NumberOfRows > 0);

	// Save line length.
	for (int32 x = 0; x < StringList.Num(); x++)
	{
		auto Element = StringList[x];
		NumberOfColumns = Element.Len();
		break;
	}

	// Reverse the StringList so that we can iterate from X=0 to X=NumberOfRows-1,
	// bottom row to top row.
	Algo::Reverse(StringList);

	// Load up our data structures that track blocking tiles.
	for (auto X = 0; X < NumberOfRows; X++)
	{
		for (auto Y = 0; Y < NumberOfColumns; Y++)
		{
			if (auto Character = StringList[X][Y]; Character == 'W' || Character == 'x')
			{
				Walls.insert(FGridLocation{X, Y});
			}
			else if (Character == '-')
			{
				OnlyGoUpTiles.insert(FGridLocation{X, Y});
			}
			else if (Character == ' ' || Character == 'o') // ' ' = dot, 'o' = no dot
			{
				// No-op.
			}
			else
			{
				check(false); // Character is not supported in our level format.
			}
		}
	}
}

void ULevelLoader::Clear()
{
	StringList.Empty();
	NumberOfRows = 0;
	NumberOfColumns = 0;
	Walls.clear();
}

int ULevelLoader::GetLevelWidth() const
{
	check(NumberOfColumns % 2 == 0);
	check(NumberOfColumns != 0);
	return NumberOfColumns;
}

int ULevelLoader::GetLevelHeight() const
{
	check(NumberOfRows % 2 == 0);
	check(NumberOfRows != 0);
	return NumberOfRows;
}

FVector2D ULevelLoader::GridToWorld(const FGridLocation& GridPosition) const
{
	FVector2D WorldPosition;
	WorldPosition.X = (static_cast<float>(GridPosition.X) - .5f * GetLevelHeight()) * 100.0f;
	WorldPosition.Y = (static_cast<float>(GridPosition.Y) - .5f * GetLevelWidth()) * 100.0f;
	return WorldPosition;
}

FGridLocation ULevelLoader::WorldToGrid(const FVector2D WorldPosition) const
{
	// Note: this is the inverse operation of GridToWorld().
	FGridLocation GridPosition;
	GridPosition.X = FMath::RoundToFloat(WorldPosition.X * .01f);
	GridPosition.Y = FMath::RoundToFloat(WorldPosition.Y * .01f);
	GridPosition.X += .5f * GetLevelHeight();
	GridPosition.Y += .5f * GetLevelWidth();
	return GridPosition;
}

FGridLocation ULevelLoader::SnapToGridDirection(const FVector2D WorldPosition)
{
	FGridLocation GridDirection{0, 0};
	const auto FractionX = fmod(WorldPosition.X, 1.0);
	const auto FractionY = fmod(WorldPosition.Y, 1.0);
	
	if (FractionX == 0.0 && FractionY == 0.0)
		return GridDirection;

	if (FractionX != 0.0)
		GridDirection.X = FractionX < 0.5 ? -1 : 1;
	
	if (FractionY != 0.0)
		GridDirection.Y = FractionY < 0.5 ? -1 : 1;

	check(GridDirection.X != GridDirection.Y);
	return GridDirection;
}

bool ULevelLoader::Passable(const FGridLocation& FromLocation, const FGridLocation& ToLocation) const
{
	if (FMath::Abs(FromLocation.X - ToLocation.X) + FMath::Abs(FromLocation.Y - ToLocation.Y) != 1)
	{
		return false;
	}

	// If ToLocation is a Wall,
	if (Walls.find(ToLocation) != Walls.end())
	{
		// Then ToLocation is not passable.
		return false;
	}

	return true;
}

FComputeTargetTileResult ULevelLoader::ComputeTargetTile(
	UWorld* World,
	FVector Location,
	FGridLocation Direction,
	const TArray<FName>& TagsToCollideWith,
	FString DebugLabel) const
{
	// Compute the actor's collision sphere. Diameter needs to be slightly less than 100.0f to avoid overlapping.
	auto ActorDiameter = 90.0f;
	auto ActorRadius = ActorDiameter * 0.5f;
	auto ActorSphere = FCollisionShape::MakeSphere(ActorRadius);

	// Given the current Position and Direction, compute the target position, but do not set the TargetTile reference just yet.
	FVector StartPos = Location;
	FVector TargetPos = Location;
	TargetPos.X += Direction.X * ActorDiameter;
	TargetPos.Y += Direction.Y * ActorDiameter;

	// Prep our result struct.
	FComputeTargetTileResult Result;

	// Perform a sweep to check if we will overlap with the target.
	TArray<FHitResult> HitResults;
	World->SweepMultiByChannel(HitResults, StartPos, TargetPos, FQuat::Identity, ECC_Visibility, ActorSphere);
	for (auto HitResult : HitResults)
	{
		// Some values for breakpoint debugging convenience.
		const auto ReadableName = HitResult.GetActor()->GetHumanReadableName();
		const auto WorldLocation = HitResult.GetActor()->GetActorLocation();
		FVector2D Location2D{WorldLocation.X, WorldLocation.Y};
		const auto GridPos = WorldToGrid(Location2D);
		const auto Blah = DebugLabel;
		
		// If we overlapped with a collider, then we can't travel to target position. Return false.
		if (auto HitActor = HitResult.GetActor(); FChompGameplayTag::ActorHasOneOf(HitActor, TagsToCollideWith))
		{
			Result.IsValid = false;
			return Result;
		}
	}

	// Otherwise, set the TargetTile (from TargetPos) and return true.
	FVector2D TargetPos2D{TargetPos.X, TargetPos.Y};
	Result.IsValid = true;
	Result.Tile = WorldToGrid(TargetPos2D);
	return Result;
}

bool ULevelLoader::InBounds(const FGridLocation& GridPosition) const
{
	return 0 <= GridPosition.X
		&& GridPosition.X < GetLevelHeight()
		&& 0 <= GridPosition.Y
		&& GridPosition.Y < GetLevelWidth();
}

std::array<FGridLocation, 4> ULevelLoader::CardinalDirections = {
	FGridLocation{1, 0}, // North
	FGridLocation{-1, 0}, // South
	FGridLocation{0, -1}, // East
	FGridLocation{0, 1} // West
};

// TODO: Add support for wraparound positions in this Neighbors function.
// You can probably pass in a set of special wraparound positions.
// Currently ghosts will never traverse to the wraparound positions.
std::vector<FGridLocation> ULevelLoader::Neighbors(const FGridLocation GridPosition) const
{
	std::vector<FGridLocation> Results;

	for (const auto [X, Y] : CardinalDirections)
	{
		FGridLocation Current{GridPosition.X, GridPosition.Y};
		if (FGridLocation Next{GridPosition.X + X, GridPosition.Y + Y}; InBounds(Next) && Passable(Current, Next))
			Results.push_back(Next);
	}

	// Apparently we need this for ordering reasons.
	//
	// See this link for details: https://www.redblobgames.com/pathfinding/a-star/implementation.html#troubleshooting-ugly-path
	if ((GridPosition.X + GridPosition.Y) % 2 == 0)
		std::reverse(Results.begin(), Results.end());

	return Results;
}

double ULevelLoader::Cost(FGridLocation FromNode, FGridLocation ToNode) const
{
	check(FMath::Abs(FromNode.X - ToNode.X) + FMath::Abs(FromNode.Y - ToNode.Y) == 1);
	return 1.0; // Arbitrary non-zero constant.
}
