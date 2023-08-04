#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(GhostHouseQueueTest, "Private.Controllers.GhostHouseQueueTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool GhostHouseQueueTest::RunTest(const FString& Parameters)
{
	{
		TArray TestArray{1, 2, 3, 4};
		const auto Actual = TestArray.Pop();
		constexpr auto Expected = 4;
		const auto Message = FString::Printf(TEXT("Actual %d = Expected %d."), Actual, Expected);
		TestEqual(Message, Actual, Expected);
		TestEqual(TEXT("Length of TestArray is now 3."), TestArray.Num(), 3);
	}

	return true;
}
