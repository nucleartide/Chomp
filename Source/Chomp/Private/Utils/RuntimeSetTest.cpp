#include "RuntimeSet.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(RuntimeSetTest, "Private.Utils.RuntimeSetTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool RuntimeSetTest::RunTest(const FString& Parameters)
{
	// Testing behavior of sorting functions.
	// It seems I gotta return the lesser of the two when comparing!
	TArray TestArray{1, 10, 100, 1000, 10000};
	Algo::Sort(TestArray, [](const int A, const int B)
	{
		return A < B;
	});

	TArray ExpectedResult{1, 10, 100, 1000, 10000};
	for (auto i = 0; i < ExpectedResult.Num(); i++)
	{
		const auto Actual = TestArray[i];
		const auto Expected = ExpectedResult[i];
		const auto Message = FString::Printf(TEXT("Actual %d = Expected %d"), Actual, Expected);
		TestEqual(Message, Actual, Expected);
	}

	// Make the test pass by returning true, or fail by returning false.
	return true;
}
