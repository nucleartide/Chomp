#include "MovementPath.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MovementPathTest, "Private.AStar.MovementPathTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MovementPathTest::RunTest(const FString& Parameters)
{
	class FTestLevelLoader : public ILevelLoader
	{
		const int Width = 30;
		const int Height = 28;

	public:
		~FTestLevelLoader() = default;
		
		virtual FVector2D GridToWorld(const FGridLocation& GridPosition) const override
		{
			FVector2D WorldPosition;
			WorldPosition.X = (static_cast<double>(GridPosition.X) - 0.5 * Height) * 100.0f;
			WorldPosition.Y = (static_cast<double>(GridPosition.Y) - 0.5 * Width) * 100.0f;
			return WorldPosition;
		}

		virtual bool IsWall(const FGridLocation& Location) const override
		{
			return false;
		}
	};

	const auto TestLevelLoader = MakeShared<FTestLevelLoader>();
	{
		const auto Actual = TestLevelLoader->GridToWorld(FGridLocation{0, 0});
		TestEqual(TEXT("GridToWorld works"), Actual, FVector2D{-1400.0, -1500.0});
	}

	{
		const TArray<FGridLocation> LocationPath{
			FGridLocation{0, 0},
		};
		const FVector CurrentLocation{-1399.99999, -1500.0, 0.0};
		const auto TestPath = FMovementPath(
			CurrentLocation,
			LocationPath,
			TestLevelLoader
		);
		const auto NextLocation = TestPath.MoveAlongPath(
			CurrentLocation,
			0.000001
		);
		const auto Dir = (CurrentLocation - NextLocation).GetSafeNormal();
		TestTrue(TEXT("Direction can be zero vector"), Dir.IsNearlyZero());
	}

	// Make the test pass by returning true, or fail by returning false.
	return true;
}
