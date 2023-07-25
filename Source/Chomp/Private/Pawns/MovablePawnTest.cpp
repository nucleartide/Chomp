#include "MovablePawnLogic.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MovablePawnTest, "Private.Pawns.MovablePawnTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MovablePawnTest::RunTest(const FString& Parameters)
{
	// Prepare object under test.
	const auto ObjectUnderTest = NewObject<UMovablePawnLogic>();

	// Prepare arguments.
	constexpr FGridLocation TargetGridPosition{0, 0};
	constexpr FGridLocation TargetDirection{0, 0};
	constexpr auto DeltaTime = 0.1f;
	const FVector Location{0.0f, 0.0f, 0.0f};
	const FRotator Rotation = FRotator::ZeroRotator;

	// Perform test.
	const auto [IsValid, Thing] = ObjectUnderTest->MoveTowardsPoint(
		TargetGridPosition,
		TargetDirection,
		DeltaTime,
		Location,
		Rotation);

	// Check assertions.
	TestTrue(TEXT("blah"), false);
	TestEqual(TEXT("blah"), Thing, 42);

	// Make the test pass by returning true, or fail by returning false.
	return true;
}
