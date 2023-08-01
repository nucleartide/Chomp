#include "Misc/AutomationTest.h"
#include "Utils/MathHelpers.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MathHelpersTest, "Private.Utils.MathHelpersTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MathHelpersTest::RunTest(const FString& Parameters)
{
	{
		constexpr auto Expected = 200.0;
		const auto Actual = MathHelpers::NotStupidFmod(-100.0, 300.0);
		TestEqual(TEXT("first"), Actual, Expected);
	}

	{
		constexpr auto Expected = 200.0;
		const auto Actual = MathHelpers::NotStupidFmod(200.0, 300.0);
		TestEqual(TEXT("second"), Actual, Expected);
	}

	{
		constexpr auto Expected = 200.0;
		const auto Actual = MathHelpers::NotStupidFmod(500.0, 300.0);
		TestEqual(TEXT("third"), Actual, Expected);
	}

	{
		constexpr auto Expected = 200.0;
		const auto Actual = MathHelpers::NotStupidFmod(-400.0, 300.0);
		TestEqual(TEXT("fourth"), Actual, Expected);
	}

	{
		constexpr auto Expected = 0.0;
		const auto Actual = MathHelpers::NotStupidFmod(-400.0, 100.0);
		TestEqual(TEXT("fifth"), Actual, Expected);
	}

	// Make the test pass by returning true, or fail by returning false.
	return true;
}
