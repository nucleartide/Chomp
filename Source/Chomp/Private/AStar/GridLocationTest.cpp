#include "GridLocation.h"
#include "LevelGenerator/ILevelLoader.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(GridLocationTest, "Private.AStar.GridLocationTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool GridLocationTest::RunTest(const FString& Parameters)
{
	class FTestLevelLoader : public ILevelLoader
	{
		const int Width = 30;
		const int Height = 28;

	public:
		virtual ~FTestLevelLoader() = default;

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

		virtual int GetLevelWidth() const override
		{
			return 30;
		}

		virtual int GetLevelHeight() const override
		{
			return 28;
		}
	};

	{
		// Setup and teardown.
		const auto TestLevelLoader = MakeShareable(new FTestLevelLoader);

		struct FUnitTest
		{
			FVector Location;
			FVector A;
			FVector B;
			std::optional<double> Expected;
		};

		const std::vector<FUnitTest> Tests{
			{
				FVector(1349.0, 0.0, 0.0),
				FVector(1300.0, 0.0, 0.0),
				FVector(-1400.0, 0.0, 0.0),
				49.0,
			},
			{
				FVector(0.0, -1525.0, 0.0),
				FVector(0.0, 1400.0, 0.0),
				FVector(0.0, -1500.0, 0.0),
				75.0,
			},
			{
				FVector(100.0, 0.0, 0.0),
				FVector(0.0, 0.0, 0.0),
				FVector(100.0, 0.0, 0.0),
				std::nullopt,
			},
			{
				FVector(0.0, 0.0, 0.0),
				FVector(100.0, 0.0, 0.0),
				FVector(0.0, 0.0, 0.0),
				std::nullopt,
			},
			{
				FVector(0.0, 0.0, 0.0),
				FVector(0.0, 0.0, 0.0),
				FVector(100.0, 0.0, 0.0),
				0.0,
			},
			{
				FVector(100.0, 0.0, 0.0),
				FVector(100.0, 0.0, 0.0),
				FVector(0.0, 0.0, 0.0),
				0.0,
			},
			{
				FVector(0.0, 1449.0, 0.0),
				FVector(0.0, -1500.0, 0.0),
				FVector(0.0, 1400.0, 0.0),
				51.0,
			},
		};

		// ReSharper disable once CppUseStructuredBinding
		for (const auto& Test : Tests)
		{
			const auto IsInBetween = FGridLocation::IsInBetween(
				Test.Location,
				Test.A,
				Test.B,
				TestLevelLoader.Object
			);
			TestEqual(
				FString::Printf(
					TEXT("%s is in between %s and %s, actual %lf, expected %lf"),
					*Test.Location.ToString(),
					*Test.A.ToString(),
					*Test.B.ToString(),
					IsInBetween.has_value() ? IsInBetween.value() : std::numeric_limits<double>::max(),
					Test.Expected.has_value() ? Test.Expected.value() : std::numeric_limits<double>::max()
				),
				IsInBetween,
				Test.Expected
			);
		}
	}

	// Make the test pass by returning true, or fail by returning false.
	return true;
}
