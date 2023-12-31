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
		const auto TestLevelLoader = MakeShareable(new FTestLevelLoader);
		const auto Actual = TestLevelLoader.Object->GridToWorld(FGridLocation{0, 0});
		TestEqual(TEXT("GridToWorld works"), Actual, FVector2D{-1400.0, -1500.0});
	}

	{
		const TArray<FGridLocation> LocationPath{
			FGridLocation{0, 0},
		};
		const FVector CurrentLocation{-1399.99999, -1500.0, 0.0};
		const auto TestLevelLoader = MakeShareable(new FTestLevelLoader);
		const auto TestPath = FMovementPath(
			CurrentLocation,
			LocationPath,
			TestLevelLoader.Object
		);
		const auto NextLocation = TestPath.MoveAlongPath(
			CurrentLocation,
			0.000001
		);
		const auto Dir = (CurrentLocation - NextLocation).GetSafeNormal();
		TestTrue(TEXT("Direction can be zero vector"), Dir.IsNearlyZero());
	}

	{
		struct FUnitTest
		{
			FVector Actual;
			FVector Expected;
		};

		const FVector A{0.0, -1500.0, 0.0};
		const FVector B{0.0, 1400.0, 0.0};
		const FVector C{-1400.0, 0.0, 0.0};
		const FVector D{1300.0, 0.0, 0.0};
		const auto TestLevelLoader = MakeShareable(new FTestLevelLoader);

		// ReSharper disable once CppTooWideScopeInitStatement
		const std::vector Tests{
			// A to B
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(A, B, 0.0, TestLevelLoader.Object),
				FVector(0.0, -1500.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(A, B, 0.25, TestLevelLoader.Object),
				FVector(0.0, -1525.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(A, B, 0.5, TestLevelLoader.Object),
				FVector(0.0, -1550.0, 0.0), // doesn't flip until threshold is exceeded
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(A, B, 0.75, TestLevelLoader.Object),
				FVector(0.0, 1425.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(A, B, 1.0, TestLevelLoader.Object),
				FVector(0.0, 1400.0, 0.0),
			},

			// B to A
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(B, A, 0.0, TestLevelLoader.Object),
				FVector(0.0, 1400.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(B, A, 0.25, TestLevelLoader.Object),
				FVector(0.0, 1425.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(B, A, 0.5, TestLevelLoader.Object),
				FVector(0.0, -1550.0, 0.0), // doesn't flip until threshold is exceeded
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(B, A, 0.75, TestLevelLoader.Object),
				FVector(0.0, -1525.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(B, A, 1.0, TestLevelLoader.Object),
				FVector(0.0, -1500.0, 0.0),
			},

			// D to C
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(D, C, 0.0, TestLevelLoader.Object),
				FVector(1300.0, 0.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(D, C, 0.25, TestLevelLoader.Object),
				FVector(1325.0, 0.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(D, C, 0.5, TestLevelLoader.Object),
				FVector(-1450.0, 0.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(D, C, 0.75, TestLevelLoader.Object),
				FVector(-1425.0, 0.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(D, C, 1.0, TestLevelLoader.Object),
				FVector(-1400.0, 0.0, 0.0),
			},

			// C to D
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(C, D, 0.0, TestLevelLoader.Object),
				FVector(-1400.0, 0.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(C, D, 0.25, TestLevelLoader.Object),
				FVector(-1425.0, 0.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(C, D, 0.5, TestLevelLoader.Object),
				FVector(-1450.0, 0.0, 0.0), // doesn't flip until threshold is exceeded
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(C, D, 0.75, TestLevelLoader.Object),
				FVector(1325.0, 0.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(C, D, 1.0, TestLevelLoader.Object),
				FVector(1300.0, 0.0, 0.0),
			},

			// happy path case
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(FVector(0.0, 100.0, 0.0), FVector(0.0, 200.0, 0.0), 0.66, TestLevelLoader.Object),
				FVector(0.0, 166.0, 0.0),
			},
			FUnitTest{
				FMovementPath::WrapFriendlyLerp(FVector(-50.0, 0.0, 0.0), FVector(50.0, 0.0, 0.0), 0.5, TestLevelLoader.Object),
				FVector(0.0, 0.0, 0.0),
			},
		};

		for (const auto& [Actual, Expected] : Tests)
		{
			const auto Message = FString::Printf(TEXT("%s == %s"), *Actual.ToString(), *Expected.ToString());
			TestEqual(Message, Actual, Expected);
		}
	}

	// Make the test pass by returning true, or fail by returning false.
	return true;
}
