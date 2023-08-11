#include "ArrayHelpers.h"
#include "Misc/AutomationTest.h"
#include <vector>

#include "Debug.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArrayHelpersTest, "Private.Utils.ArrayHelpersTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool ArrayHelpersTest::RunTest(const FString& Parameters)
{
	{
		TArray SomeArray{1, 2, 3, 4, 5};
		FArrayHelpers::Randomize(SomeArray);
	}

	{
		std::vector SomeArray{1, 2, 3, 4, 5};
		FArrayHelpers::Randomize(SomeArray);
		DEBUG_LOG(TEXT("breakpoint"));
	}

	// Make the test pass by returning true, or fail by returning false.
	return true;
}
